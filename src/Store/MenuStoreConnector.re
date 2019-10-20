/*
 * MenuStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Menu
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module Quickmenu = Model.Quickmenu;
module MenuJob = Model.MenuJob;
module Utility = Core.Utility;


// TODO: Remove after 4.08 upgrade
module Option = {
  let map = f => fun
    | Some(x) => Some(f(x))
    | None => None

  let value = (~default) => fun
    | Some(x) => x
    | None => default

  let some = x =>
    Some(x)
};


let prefixFor : Vim.Types.cmdlineType => string = fun 
  | SearchForward => "/"
  | SearchReverse => "?"
  | _ => ":"


let start = () => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let selectItemEffect = command =>
    Isolinear.Effect.createWithDispatch(~name="menu.selectItem", dispatch => {
      let action = command();
      dispatch(action);
    });

  let executeVimCommandEffect =
    Isolinear.Effect.createWithDispatch(~name="menu.executeVimCommand", dispatch => {
      // TODO: Hard-coding "<CR>" and assuming `KeyboardInput` reaches vim seems very sketchy
      dispatch(Actions.KeyboardInput("<CR>"));
    });

  let exitModeEffect =
    Isolinear.Effect.createWithDispatch(~name="menu.exitMode", dispatch => {
      // TODO: Hard-coding "<ESC>" and assuming `KeyboardInput` reaches vim seems very sketchy
      dispatch(Actions.KeyboardInput("<ESC>"));
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
            Some(Actions.{
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
              highlight: []
            })
          | None => None
          };
        })
    |> Array.of_seq;
  };

  let menuUpdater = (state: option(Quickmenu.t), action: Actions.t, buffers, languageInfo, iconTheme)
    : (option(Quickmenu.t), Isolinear.Effect.t(Actions.t)) => {
    switch (action) {
    | MenuShow(CommandPalette) =>
      (
        Some{{
          ...Quickmenu.defaults(CommandPalette),
          source: Complete(Model.CommandPalette.commands),
          selected: Some(0)
        }},
        Isolinear.Effect.none
      );

    | MenuShow(Buffers) =>
      (
        Some{{
          ...Quickmenu.defaults(Buffers),
          source: Complete(makeBufferCommands(languageInfo, iconTheme, buffers)),
          selected: Some(0)
        }},
        Isolinear.Effect.none
      );

    | MenuShow(WorkspaceFiles) =>
      (
        Some{{
          ...Quickmenu.defaults(WorkspaceFiles),
          source: Loading,
          selected: Some(0)
        }},
        Isolinear.Effect.none
      );

    | MenuShow(Wildmenu(cmdType)) =>
      (
        Some{{
          ...Quickmenu.defaults(Wildmenu(cmdType)),
          prefix: Some(prefixFor(cmdType)),
        }},
        Isolinear.Effect.none
      );

    | MenuInput({ text, cursorPosition }) =>
      (
        Option.map(state => Quickmenu.{ ...state, text, cursorPosition }, state),
        Isolinear.Effect.none
      );

    | MenuUpdateSource(source) =>
      (
        Option.map((state: Quickmenu.t) => {
          let count = Quickmenu.getCount(source);
          {...state, source, selected: Option.map(min(count), state.selected)}
        }, state),
        Isolinear.Effect.none
      );

    | MenuFocus(index) => (
      Option.map((state: Quickmenu.t) => {
        let count = Quickmenu.getCount(state.source);

        {
          ...state,
          selected: Some(Utility.clamp(index, ~lo=0, ~hi=count))
        }
      }, state),
      Isolinear.Effect.none
    )

    | NotifyKeyPressed(_, "<UP>")
    | MenuFocusPrevious => (
      Option.map((state: Quickmenu.t) => {
        let count = Quickmenu.getCount(state.source);

        {
          ...state,
          selected:
            Option.map(selected =>
              if (count == 0) {
                0
              } else if (selected <= 0) {
                count - 1 // "roll over" to end of list
              } else {
                selected - 1
              }, state.selected)
            |> Option.value(~default=max(count -1, 0)) // default to end of list
            |> Option.some
        }
      }, state),
      Isolinear.Effect.none
    )

    | NotifyKeyPressed(_, "<DOWN>")
    | MenuFocusNext => (
      Option.map((state: Quickmenu.t) => {
        let count = Quickmenu.getCount(state.source);

        {
          ...state,
          selected:
            Option.map(selected =>
              if (count == 0) {
                0
              } else {
                (selected + 1) mod count
              }, state.selected)
            |> Option.value(~default=0) // default to start of list
            |> Option.some
        }
      }, state),
      Isolinear.Effect.none
    )

    | MenuSelect =>
      switch (state) {
        | Some({ variant: Wildmenu(_) }) =>
          (None, executeVimCommandEffect)

        | Some({ source, selected: Some(selected) }) =>
          let items = Quickmenu.getItems(source);
          switch (items[selected]) {
          | v =>
            (None, selectItemEffect(v.command))

          | exception Invalid_argument(_) =>
            (state, Isolinear.Effect.none)
          }

        | _ =>
          (state, Isolinear.Effect.none)
      }

    | MenuClose =>
      switch (state) {
      | Some({ variant: Wildmenu(_) }) =>
        (None, exitModeEffect);
      | _ =>
        (None, Isolinear.Effect.none);
      }

    | _ => (state, Isolinear.Effect.none)
    };
  };

  let updater = (state: Model.State.t, action: Actions.t) => {
    let (menuState, menuEffect) = menuUpdater(state.quickmenu, action, state.buffers, state.languageInfo, state.iconTheme);
    let state = {...state, quickmenu: menuState};
    (state, menuEffect);
  };

  (updater, stream);
};


let subscriptions = (ripgrep) => {
  let (stream, dispatch) = Isolinear.Stream.create();
  let (itemStream, addItems) = Isolinear.Stream.create();

  let filter = (query, source) => {
    MenuJobSubscription.create(
      ~id="menu-filter",
      ~query,
      ~items  = Quickmenu.getItems(source) |> Array.to_list, // TODO: This doesn't seem very efficient. Can Array.to_list be removed?
      ~itemStream,
      ~onUpdate=(items, ~progress) =>
        Actions.MenuUpdateSource(progress == 1. ? Complete(items) : Progress({ items, progress }))
    );
  };

  let ripgrep = (languageInfo, iconTheme) => {
    let directory = Rench.Environment.getWorkingDirectory();

    let re = Str.regexp_string(directory ++ Filename.dir_sep);

    let getDisplayPath = fullPath =>
      Str.replace_first(re, "", fullPath);

    let stringToCommand =
      (languageInfo, iconTheme, fullPath) => Actions.{
        category: None,
        name: getDisplayPath(fullPath),
        command: () => Model.Actions.OpenFileByPath(fullPath, None),
        icon:
          Model.FileExplorer.getFileIcon(languageInfo, iconTheme, fullPath),
        highlight: []
      };

    RipgrepSubscription.create(
      ~id="workspace-search",
      ~directory, ~ripgrep,
      ~onUpdate=items => List.map(stringToCommand(languageInfo, iconTheme), items) |> addItems,
      ~onCompleted=() => Noop
    );
  };

  let updater = (state: Model.State.t) => {
    switch (state.quickmenu) {
      | Some(menu) =>
        switch (menu.variant) {
          | CommandPalette
          | Buffers =>
            [filter(menu.text, menu.source)]

          | WorkspaceFiles =>
            [filter(menu.text, menu.source), ripgrep(state.languageInfo, state.iconTheme)]

          | Wildmenu(_) =>
            []
        }

      | None =>
        []
    };
  };

  (updater, stream)
}