/*
 * QuickmenuStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Quickmenu
 */
open Oni_Core;
open Oni_Model;
open Oni_UI;
open Utility;

module ExtensionContributions = Exthost.Extension.Contributions;

module Log = (val Log.withNamespace("Oni2.Store.Quickmenu"));

module Internal = {
  let prefixFor: Vim.Types.cmdlineType => string =
    fun
    | SearchForward => "/"
    | SearchReverse => "?"
    | Ex
    | Unknown => ":";

  let commandsToMenuItems = (commands, items) =>
    items
    |> List.map((item: Menu.item) =>
         Actions.{
           category: item.category,
           name: item.label,
           command: _ =>
             switch (Command.Lookup.get(item.command, commands)) {
             | Some({msg: `Arg0(msg), _}) => msg
             | Some({msg: `Arg1(msgf), _}) => msgf(Json.Encode.null)
             | None => Actions.Noop
             },
           icon: item.icon,
           highlight: [],
         }
       )
    |> Array.of_list;

  let commandPaletteItems = (commands, menus, contextKeys) => {
    let contextKeys =
      WhenExpr.ContextKeys.union(
        contextKeys,
        WhenExpr.ContextKeys.fromList([
          (
            "oni.symLinkExists",
            WhenExpr.Value.(
              Sys.file_exists("/usr/local/bin/oni2") ? True : False
            ),
          ),
        ]),
      );

    Feature_Menus.commandPalette(contextKeys, commands, menus)
    |> commandsToMenuItems(commands);
  };
};

let start = () => {
  let selectItemEffect = (~dir=None, item: Actions.menuItem) =>
    Isolinear.Effect.createWithDispatch(~name="quickmenu.selectItem", dispatch => {
      let action = item.command(dir);
      dispatch(action);
    });

  let executeVimCommandEffect =
    Isolinear.Effect.createWithDispatch(
      ~name="quickmenu.executeVimCommand", dispatch => {
      // TODO: Hard-coding "<CR>" and assuming `KeyboardInput` reaches vim seems very sketchy
      dispatch(
        Actions.KeyboardInput({isText: false, input: "<CR>"}),
      )
    });

  let exitModeEffect =
    Isolinear.Effect.createWithDispatch(~name="quickmenu.exitMode", dispatch => {
      // TODO: Hard-coding "<ESC>" and assuming `KeyboardInput` reaches vim seems very sketchy
      dispatch(
        Actions.KeyboardInput({isText: false, input: "<ESC>"}),
      )
    });

  let makeBufferCommands =
      (layout, workspace, languageInfo, iconTheme, buffers) => {
    // Get visible buffers

    let bufferIds: IntSet.t =
      layout
      |> Feature_Layout.activeLayoutGroups
      |> List.map(Feature_Layout.Group.allEditors)
      |> List.flatten
      |> List.map(Feature_Editor.Editor.getBufferId)
      |> List.fold_left(
           (acc, curr) => {IntSet.add(curr, acc)},
           IntSet.empty,
         );
    // Get the current workspace, if available
    let maybeWorkspace = Feature_Workspace.openedFolder(workspace);

    buffers
    |> Feature_Buffers.all
    |> List.filter(buffer => IntSet.mem(Buffer.getId(buffer), bufferIds))
    // Sort by most recerntly used
    |> List.fast_sort((a, b) =>
         - Float.compare(Buffer.getLastUsed(a), Buffer.getLastUsed(b))
       )
    |> List.filter_map(buffer => {
         let maybeName =
           Buffer.getMediumFriendlyName(
             ~workingDirectory=?maybeWorkspace,
             buffer,
           );
         let maybePath = Buffer.getFilePath(buffer);

         OptionEx.map2(
           (name, path) =>
             Actions.{
               category: None,
               name,
               command: _ => {
                 Actions.OpenFileByPath(path, SplitDirection.Current, None);
               },
               icon:
                 Component_FileExplorer.getFileIcon(
                   ~languageInfo,
                   ~iconTheme,
                   path,
                 ),
               highlight: [],
             },
           maybeName,
           maybePath,
         );
       })
    |> Array.of_list;
  };

  let typeToSearchInput =
    Component_InputText.create(~placeholder="type to search...");

  let menuUpdater =
      (
        state: option(Quickmenu.t),
        action: Actions.t,
        buffers,
        languageInfo,
        iconTheme,
        layout,
        workspace,
        commands,
        menus,
        contextKeys,
      )
      : (option(Quickmenu.t), Isolinear.Effect.t(Actions.t)) => {
    switch (action) {
    | QuickmenuShow(CommandPalette) => (
        Some({
          ...Quickmenu.defaults(CommandPalette),
          items: Internal.commandPaletteItems(commands, menus, contextKeys),
          focused: Some(0),
          inputText: typeToSearchInput,
        }),
        Isolinear.Effect.none,
      )

    | QuickmenuShow(EditorsPicker) =>
      let items =
        makeBufferCommands(
          layout,
          workspace,
          languageInfo,
          iconTheme,
          buffers,
        );

      (
        Some({
          ...Quickmenu.defaults(EditorsPicker),
          items,
          focused: Some(min(1, Array.length(items) - 1)),
        }),
        Isolinear.Effect.none,
      );

    | QuickmenuShow(FilesPicker) =>
      if (Feature_Workspace.openedFolder(workspace) == None) {
        let items =
          makeBufferCommands(
            layout,
            workspace,
            languageInfo,
            iconTheme,
            buffers,
          );

        let focused = items != [||] ? Some(0) : None;
        (
          Some({...Quickmenu.defaults(OpenBuffersPicker), items, focused}),
          Isolinear.Effect.none,
        );
      } else {
        (
          Some({
            ...Quickmenu.defaults(FilesPicker),
            filterProgress: Loading,
            ripgrepProgress: Loading,
            inputText: typeToSearchInput,
            focused: Some(0),
          }),
          Isolinear.Effect.none,
        );
      }

    | QuickmenuShow(Wildmenu(cmdType)) => (
        Some({
          ...Quickmenu.defaults(Wildmenu(cmdType)),
          prefix: Some(Internal.prefixFor(cmdType)),
        }),
        Isolinear.Effect.none,
      )

    | QuickmenuShow(OpenBuffersPicker) =>
      let items =
        makeBufferCommands(
          layout,
          workspace,
          languageInfo,
          iconTheme,
          buffers,
        );
      let focused = items != [||] ? Some(0) : None;

      (
        Some({...Quickmenu.defaults(OpenBuffersPicker), items, focused}),
        Isolinear.Effect.none,
      );

    | QuickmenuPaste(text) => (
        Option.map(
          (Quickmenu.{inputText, _} as state) => {
            let inputText = Component_InputText.paste(~text, inputText);

            Quickmenu.{...state, inputText, focused: Some(0)};
          },
          state,
        ),
        Isolinear.Effect.none,
      )
    | QuickmenuInput(key) => (
        Option.map(
          (Quickmenu.{inputText, _} as state) => {
            let inputText = Component_InputText.handleInput(~key, inputText);

            Quickmenu.{...state, inputText, focused: Some(0)};
          },
          state,
        ),
        Isolinear.Effect.none,
      )

    | QuickmenuInputMessage(msg) => (
        Option.map(
          (Quickmenu.{variant, inputText, _} as state) => {
            switch (variant) {
            | Wildmenu(_) =>
              let oldPosition =
                inputText |> Component_InputText.cursorPosition;

              let (inputText, _) =
                Component_InputText.update(msg, inputText);
              let newPosition =
                inputText |> Component_InputText.cursorPosition;
              let transition = newPosition - oldPosition;

              if (transition > 0) {
                for (_ in 0 to transition) {
                  GlobalContext.current().dispatch(
                    Actions.KeyboardInput({isText: false, input: "<LEFT>"}),
                  );
                };
              } else if (transition < 0) {
                for (_ in 0 downto transition) {
                  GlobalContext.current().dispatch(
                    Actions.KeyboardInput({isText: false, input: "<RIGHT>"}),
                  );
                };
              };
            | _ => ()
            };

            Quickmenu.{...state, variant, inputText};
          },
          state,
        ),
        Isolinear.Effect.none,
      )

    | QuickmenuCommandlineUpdated(text, cursor) => (
        Option.map(
          state =>
            Quickmenu.{
              ...state,
              inputText:
                Component_InputText.set(~text, ~cursor, state.inputText),
            },
          state,
        ),
        Isolinear.Effect.none,
      )

    | QuickmenuUpdateRipgrepProgress(progress) => (
        Option.map(
          (state: Quickmenu.t) => {...state, ripgrepProgress: progress},
          state,
        ),
        Isolinear.Effect.none,
      )

    | QuickmenuUpdateFilterProgress(items, progress) => (
        Option.map(
          (state: Quickmenu.t) => {
            let count = Array.length(items);
            {
              ...state,
              items,
              filterProgress: progress,
              focused: Option.map(min(count), state.focused),
            };
          },
          state,
        ),
        Isolinear.Effect.none,
      )

    | ListFocus(index) => (
        Option.map(
          (state: Quickmenu.t) => {
            let count = Array.length(state.items);

            {...state, focused: Some(IntEx.clamp(index, ~lo=0, ~hi=count))};
          },
          state,
        ),
        Isolinear.Effect.none,
      )

    | ListFocusUp => (
        Option.map(
          (state: Quickmenu.t) =>
            {
              ...state,
              focused:
                IndexEx.prevRollOverOpt(
                  state.focused,
                  ~last=Array.length(state.items) - 1,
                ),
            },
          state,
        ),
        Isolinear.Effect.none,
      )

    | ListFocusDown => (
        Option.map(
          (state: Quickmenu.t) =>
            {
              ...state,
              focused:
                IndexEx.nextRollOverOpt(
                  state.focused,
                  ~last=Array.length(state.items) - 1,
                ),
            },
          state,
        ),
        Isolinear.Effect.none,
      )

    | ListSelect(args) =>
      switch (state) {
      | Some({variant: Wildmenu(_), _}) => (None, executeVimCommandEffect)

      | Some({items, focused: Some(focused), _}) =>
        switch (items[focused]) {
        | item => (None, selectItemEffect(~dir=Some(args.direction), item))
        | exception (Invalid_argument(_)) => (state, Isolinear.Effect.none)
        }

      | _ => (state, Isolinear.Effect.none)
      }

    | ListSelectBackground =>
      switch (state) {
      | Some({items, focused: Some(focused), _}) =>
        let eff =
          switch (items[focused]) {
          | item => selectItemEffect(item)
          | exception (Invalid_argument(_)) => Isolinear.Effect.none
          };
        (state, eff);

      | _ => (state, Isolinear.Effect.none)
      }

    | QuickmenuClose =>
      switch (state) {
      | Some({variant: Wildmenu(_), _}) => (None, exitModeEffect)
      | _ => (None, Isolinear.Effect.none)
      }

    | _ => (state, Isolinear.Effect.none)
    };
  };

  let updater = (state: State.t, action: Actions.t) =>
    // Transition menus to this new-style quickmenu
    if (Feature_Quickmenu.isMenuOpen(state.newQuickmenu)) {
      switch (action) {
      | ListFocusUp =>
        let (newQuickmenu, eff) = Feature_Quickmenu.prev(state.newQuickmenu);
        ({...state, newQuickmenu}, eff);
      | ListFocusDown =>
        let (newQuickmenu, eff) = Feature_Quickmenu.next(state.newQuickmenu);
        ({...state, newQuickmenu}, eff);
      | ListSelect(_) =>
        let (newQuickmenu, eff) =
          Feature_Quickmenu.select(state.newQuickmenu);
        ({...state, newQuickmenu}, eff);
      | QuickmenuClose => (
          {
            ...state,
            newQuickmenu: Feature_Quickmenu.cancel(state.newQuickmenu),
          },
          Isolinear.Effect.none,
        )
      | _ => (state, Isolinear.Effect.none)
      };
    } else {
      let (menuState, menuEffect) =
        menuUpdater(
          state.quickmenu,
          action,
          state.buffers,
          state.languageSupport |> Feature_LanguageSupport.languageInfo,
          state.iconTheme,
          state.layout,
          state.workspace,
          CommandManager.current(state),
          MenuManager.current(state),
          ContextKeys.all(state),
        );

      ({...state, quickmenu: menuState}, menuEffect);
    };

  updater;
};

module QuickmenuFilterSubscription =
  FilterSubscription.Make({
    type item = Actions.menuItem;
    let format = Quickmenu.getLabel;
  });

let subscriptions = (ripgrep, dispatch) => {
  let (itemStream, addItems) = Isolinear.Stream.create();

  let filter = (query, items) => {
    // HACK: Filter out spaces, so queries with spaces behave in a sane way.
    // However, this won't be needed once Fzy has the full refine behavior,
    // described in https://github.com/onivim/oni2/issues/3278
    let queryWithoutSpaces = query |> StringEx.filterAscii(c => c != ' ');
    QuickmenuFilterSubscription.create(
      ~id="quickmenu-filter",
      ~query=queryWithoutSpaces,
      ~items=items |> Array.to_list, // TODO: This doesn't seem very efficient. Can Array.to_list be removed?
      ~itemStream,
      ~onUpdate=(items, ~progress) => {
        let items =
          items
          |> List.map((Filter.{item, highlight, _}) =>
               ({...item, highlight}: Actions.menuItem)
             )
          |> Array.of_list;
        Actions.QuickmenuUpdateFilterProgress(
          items,
          progress == 1. ? Complete : InProgress(progress),
        );
      },
    );
  };

  let ripgrep = (workspace, languageInfo, iconTheme, config) => {
    let filesExclude =
      Feature_Configuration.GlobalConfiguration.Files.exclude.get(config);

    let followSymlinks =
      Feature_Configuration.GlobalConfiguration.Search.followSymlinks.get(
        config,
      );

    let useIgnoreFiles =
      Feature_Configuration.GlobalConfiguration.Search.useIgnoreFiles.get(
        config,
      );

    switch (Feature_Workspace.openedFolder(workspace)) {
    | None =>
      // It's not really clear what the behavior should be for the ripgrep subscription w/o
      // an opened workspace / folder. In the current logic, we shouldn't ever get here -
      // when opening the 'files picker', if there is no workspace, we'll open the 'buffers picker'
      // instead.
      []
    | Some(directory) =>
      let stringToCommand = (languageInfo, iconTheme, fullPath) =>
        Actions.{
          category: None,
          name: Path.toRelative(~base=directory, fullPath),
          command: splitDir =>
            switch (splitDir) {
            | Some(dir) => Actions.OpenFileByPath(fullPath, dir, None)
            | None =>
              Actions.OpenFileByPath(fullPath, SplitDirection.Current, None)
            },
          icon:
            Component_FileExplorer.getFileIcon(
              ~languageInfo,
              ~iconTheme,
              fullPath,
            ),
          highlight: [],
        };
      [
        RipgrepSubscription.create(
          ~id="workspace-search",
          ~followSymlinks,
          ~useIgnoreFiles,
          ~filesExclude,
          ~directory,
          ~ripgrep,
          ~onUpdate=
            items => {
              items
              |> List.map(stringToCommand(languageInfo, iconTheme))
              |> addItems;

              dispatch(Actions.QuickmenuUpdateRipgrepProgress(Loading));
            },
          ~onComplete=() => Actions.QuickmenuUpdateRipgrepProgress(Complete),
          ~onError=_ => Actions.Noop,
        ),
      ];
    };
  };

  let updater = (state: State.t) => {
    switch (state.quickmenu) {
    | Some(quickmenu) =>
      let query = quickmenu.inputText |> Component_InputText.value;
      switch (quickmenu.variant) {
      | CommandPalette
      | EditorsPicker
      | OpenBuffersPicker => [filter(query, quickmenu.items)]

      | FilesPicker =>
        [filter(query, quickmenu.items)]
        @ ripgrep(
            state.workspace,
            state.languageSupport |> Feature_LanguageSupport.languageInfo,
            state.iconTheme,
            state |> Oni_Model.Selectors.configResolver,
          )

      | Wildmenu(_) => []
      };

    | None => []
    };
  };

  updater;
};
