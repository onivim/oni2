/*
 * MenuStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Menu
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module Menu = Model.Menu;
module Utility = Core.Utility;
module ExtensionContributions = Oni_Extensions.ExtensionContributions;

// TODO: Remove after 4.08 upgrade
module Option = {
  let map = f =>
    fun
    | Some(x) => Some(f(x))
    | None => None;

  let value = (~default) =>
    fun
    | Some(x) => x
    | None => default;

  let some = x => Some(x);
};

let prefixFor: Vim.Types.cmdlineType => string =
  fun
  | SearchForward => "/"
  | SearchReverse => "?"
  | _ => ":";

let start = (themeInfo: Model.ThemeInfo.t) => {
  let (stream, _dispatch) = Isolinear.Stream.create();

  let selectItemEffect = (item: Actions.menuItem) =>
    Isolinear.Effect.createWithDispatch(~name="menu.selectItem", dispatch => {
      let action = item.command();
      dispatch(action);
    });

  let executeVimCommandEffect =
    Isolinear.Effect.createWithDispatch(
      ~name="menu.executeVimCommand", dispatch => {
      // TODO: Hard-coding "<CR>" and assuming `KeyboardInput` reaches vim seems very sketchy
      dispatch(
        Actions.KeyboardInput("<CR>"),
      )
    });

  let exitModeEffect =
    Isolinear.Effect.createWithDispatch(~name="menu.exitMode", dispatch => {
      // TODO: Hard-coding "<ESC>" and assuming `KeyboardInput` reaches vim seems very sketchy
      dispatch(
        Actions.KeyboardInput("<ESC>"),
      )
    });

  let makeBufferCommands = (languageInfo, iconTheme, buffers) => {
    let currentDirectory = Rench.Environment.getWorkingDirectory();

    let getDisplayPath = (fullPath, dir) => {
      let re = Str.regexp_string(dir ++ Filename.dir_sep);
      Str.replace_first(re, "", fullPath);
    };

    buffers
    |> Core.IntMap.to_seq
    |> Seq.filter_map(element => {
         let (_, buffer) = element;

         switch (Model.Buffer.getFilePath(buffer)) {
         | Some(path) =>
           Some(
             Actions.{
               category: None,
               name: getDisplayPath(path, currentDirectory),
               command: () => {
                 Oni_Model.Actions.OpenFileByPath(path, None);
               },
               icon:
                 Oni_Model.FileExplorer.getFileIcon(
                   languageInfo,
                   iconTheme,
                   path,
                 ),
               highlight: [],
             },
           )
         | None => None
         };
       })
    |> Array.of_seq;
  };

  let menuUpdater =
      (
        state: option(Menu.t),
        action: Actions.t,
        buffers,
        languageInfo,
        iconTheme,
        themeInfo,
      )
      : (option(Menu.t), Isolinear.Effect.t(Actions.t)) => {
    switch (action) {
    | MenuShow(CommandPalette) => (
        Some({
          ...Menu.defaults(CommandPalette),
          items: Model.CommandPalette.commands,
          selected: Some(0),
        }),
        Isolinear.Effect.none,
      )

    | MenuShow(Buffers) => (
        Some({
          ...Menu.defaults(Buffers),
          items: makeBufferCommands(languageInfo, iconTheme, buffers),
          selected: Some(0),
        }),
        Isolinear.Effect.none,
      )

    | MenuShow(WorkspaceFiles) => (
        Some({
          ...Menu.defaults(WorkspaceFiles),
          ripgrepProgress: Loading,
          selected: Some(0),
        }),
        Isolinear.Effect.none,
      )

    | MenuShow(Wildmenu(cmdType)) => (
        Some({
          ...Menu.defaults(Wildmenu(cmdType)),
          prefix: Some(prefixFor(cmdType)),
        }),
        Isolinear.Effect.none,
      )

    | MenuShow(Themes) =>
      let items =
        Model.ThemeInfo.getThemes(themeInfo)
        |> List.map((theme: ExtensionContributions.Theme.t) => {
             Actions.{
               category: Some("Theme"),
               name: theme.label,
               command: () => ThemeLoadByName(theme.label),
               icon: None,
               highlight: [],
             }
           })
        |> Array.of_list;

      (Some({...Menu.defaults(Themes), items}), Isolinear.Effect.none);

    | MenuInput({text, cursorPosition}) => (
        Option.map(state => Menu.{...state, text, cursorPosition}, state),
        Isolinear.Effect.none,
      )

    | MenuUpdateRipgrepProgress(progress) => (
        Option.map(
          (state: Menu.t) => {...state, ripgrepProgress: progress},
          state,
        ),
        Isolinear.Effect.none,
      )

    | MenuUpdateFilterProgress(items, progress) => (
        Option.map(
          (state: Menu.t) => {
            let count = Array.length(items);
            {
              ...state,
              items,
              filterProgress: progress,
              selected: Option.map(min(count), state.selected),
            };
          },
          state,
        ),
        Isolinear.Effect.none,
      )

    | ListFocus(index) => (
        Option.map(
          (state: Menu.t) => {
            let count = Array.length(state.items);

            {
              ...state,
              selected: Some(Utility.clamp(index, ~lo=0, ~hi=count)),
            };
          },
          state,
        ),
        Isolinear.Effect.none,
      )

    | ListFocusUp => (
        Option.map(
          (state: Menu.t) => {
            let count = Array.length(state.items);

            {
              ...state,
              selected:
                Option.map(
                  selected =>
                    if (count == 0) {
                      0;
                    } else if (selected <= 0) {
                      count - 1; // "roll over" to end of list
                    } else {
                      selected - 1;
                    },
                  state.selected,
                )
                |> Option.value(~default=max(count - 1, 0))  // default to end of list
                |> Option.some,
            };
          },
          state,
        ),
        Isolinear.Effect.none,
      )

    | ListFocusDown => (
        Option.map(
          (state: Menu.t) => {
            let count = Array.length(state.items);

            {
              ...state,
              selected:
                Option.map(
                  selected =>
                    if (count == 0) {
                      0;
                    } else {
                      (selected + 1) mod count;
                    },
                  state.selected,
                )
                |> Option.value(~default=0)  // default to start of list
                |> Option.some,
            };
          },
          state,
        ),
        Isolinear.Effect.none,
      )

    | ListSelect =>
      switch (state) {
      | Some({variant: Wildmenu(_), _}) => (None, executeVimCommandEffect)

      | Some({items, selected: Some(selected), _}) =>
        switch (items[selected]) {
        | item => (None, selectItemEffect(item))
        | exception (Invalid_argument(_)) => (state, Isolinear.Effect.none)
        }

      | _ => (state, Isolinear.Effect.none)
      }

    | ListSelectBackground =>
      switch (state) {
      | Some({items, selected: Some(selected), _}) =>
        let eff =
          switch (items[selected]) {
          | item => selectItemEffect(item)
          | exception (Invalid_argument(_)) => Isolinear.Effect.none
          };
        (state, eff);

      | _ => (state, Isolinear.Effect.none)
      }

    | MenuClose =>
      switch (state) {
      | Some({variant: Wildmenu(_), _}) => (None, exitModeEffect)
      | _ => (None, Isolinear.Effect.none)
      }

    | _ => (state, Isolinear.Effect.none)
    };
  };

  let updater = (state: Model.State.t, action: Actions.t) => {
    switch (action) {
    | Tick(_) => (state, Isolinear.Effect.none)

    | action =>
      let (menuState, menuEffect) =
        menuUpdater(
          state.menu,
          action,
          state.buffers,
          state.languageInfo,
          state.iconTheme,
          themeInfo,
        );

      ({...state, menu: menuState}, menuEffect);
    };
  };

  (updater, stream);
};

let subscriptions = ripgrep => {
  let (stream, dispatch) = Isolinear.Stream.create();
  let (itemStream, addItems) = Isolinear.Stream.create();

  module MenuFilterSubscription =
    FilterSubscription.Make({
      type item = Actions.menuItem;
      let format = Model.Menu.getLabel;
    });

  let filter = (query, items) => {
    MenuFilterSubscription.create(
      ~id="menu-filter",
      ~query,
      ~items=items |> Array.to_list, // TODO: This doesn't seem very efficient. Can Array.to_list be removed?
      ~itemStream,
      ~onUpdate=(items, ~progress) => {
        let items =
          items
          |> List.map((Model.Filter.{item, highlight}) =>
               ({...item, highlight}: Actions.menuItem)
             )
          |> Array.of_list;
        Actions.MenuUpdateFilterProgress(
          items,
          progress == 1. ? Complete : InProgress(progress),
        );
      },
    );
  };

  let ripgrep = (languageInfo, iconTheme) => {
    let directory = Rench.Environment.getWorkingDirectory();

    let re = Str.regexp_string(directory ++ Filename.dir_sep);

    let getDisplayPath = fullPath => Str.replace_first(re, "", fullPath);

    let stringToCommand = (languageInfo, iconTheme, fullPath) =>
      Actions.{
        category: None,
        name: getDisplayPath(fullPath),
        command: () => Model.Actions.OpenFileByPath(fullPath, None),
        icon:
          Model.FileExplorer.getFileIcon(languageInfo, iconTheme, fullPath),
        highlight: [],
      };

    RipgrepSubscription.create(
      ~id="workspace-search",
      ~directory,
      ~ripgrep,
      ~onUpdate=
        items => {
          items
          |> List.map(stringToCommand(languageInfo, iconTheme))
          |> addItems;

          dispatch(Actions.MenuUpdateRipgrepProgress(Loading));
        },
      ~onCompleted=() => Actions.MenuUpdateRipgrepProgress(Complete),
    );
  };

  let updater = (state: Model.State.t) => {
    switch (state.menu) {
    | Some(menu) =>
      switch (menu.variant) {
      | CommandPalette
      | Buffers
      | Themes => [filter(menu.text, menu.items)]

      | WorkspaceFiles => [
          filter(menu.text, menu.items),
          ripgrep(state.languageInfo, state.iconTheme),
        ]

      | Wildmenu(_) => []
      }

    | None => []
    };
  };

  (updater, stream);
};
