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
           command: () =>
             switch (Command.Lookup.get(item.command, commands)) {
             | Some({msg: `Arg0(msg), _}) => msg
             | Some({msg: `Arg1(msgf), _}) => msgf(Json.Encode.null)
             | None => Actions.Noop
             },
           icon: item.icon,
           highlight: [],
           handle: None,
         }
       )
    |> Array.of_list;

  let extensionItem: Exthost.QuickOpen.Item.t => Actions.menuItem =
    item => {
      category: None,
      name: item.label,
      handle: Some(item.handle),
      icon: None,
      highlight: [],
      command: () => Actions.Noop,
    };

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
  let selectItemEffect = (item: Actions.menuItem) =>
    Isolinear.Effect.createWithDispatch(~name="quickmenu.selectItem", dispatch => {
      let action = item.command();
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

  exception MenuCancelled;

  let selectExtensionItemEffect = (~resolver, item: Actions.menuItem) =>
    Isolinear.Effect.create(~name="quickmenu.selectExtensionItem", () => {
      switch (item.handle) {
      | Some(handle) => Lwt.wakeup(resolver, handle)
      | None => Lwt.wakeup_exn(resolver, MenuCancelled)
      }
    });

  let cancelExtensionMenuEffect = (~resolver) =>
    Isolinear.Effect.create(~name="quickmenu.cancelExtensionMenu", () => {
      Lwt.wakeup_exn(resolver, MenuCancelled)
    });

  let makeBufferCommands = (workspace, languageInfo, iconTheme, buffers) => {
    // Get the current workspace, if available
    let maybeWorkspace = Feature_Workspace.openedFolder(workspace);

    buffers
    |> Feature_Buffers.all
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
               command: () => {
                 Actions.OpenFileByPath(path, None, None);
               },
               icon:
                 Component_FileExplorer.getFileIcon(
                   ~languageInfo,
                   ~iconTheme,
                   path,
                 ),
               highlight: [],
               handle: None,
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
        makeBufferCommands(workspace, languageInfo, iconTheme, buffers);

      (
        Some({
          ...Quickmenu.defaults(EditorsPicker),
          items,
          focused: Some(min(1, Array.length(items) - 1)),
        }),
        Isolinear.Effect.none,
      );

    | QuickmenuShow(Extension({id, hasItems, resolver})) => (
        Some({
          ...Quickmenu.defaults(Extension({id, hasItems, resolver})),
          focused: Some(0),
        }),
        Isolinear.Effect.none,
      )

    | QuickmenuShow(FilesPicker) =>
      if (Feature_Workspace.openedFolder(workspace) == None) {
        let items =
          makeBufferCommands(workspace, languageInfo, iconTheme, buffers);

        (
          Some({...Quickmenu.defaults(OpenBuffersPicker), items}),
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

    | QuickmenuShow(ThemesPicker(themes)) =>
      let items =
        themes
        |> List.map((theme: ExtensionContributions.Theme.t) => {
             Actions.{
               category: Some("Theme"),
               name: ExtensionContributions.Theme.label(theme),
               command: () =>
                 ThemeLoadById(ExtensionContributions.Theme.id(theme)),
               icon: None,
               highlight: [],
               handle: None,
             }
           })
        |> Array.of_list;

      (
        Some({...Quickmenu.defaults(ThemesPicker(themes)), items}),
        Isolinear.Effect.none,
      );

    | QuickmenuShow(SnippetPicker(snippets)) =>
      let items =
        snippets
        |> List.map((snippet: Service_Snippets.SnippetWithMetadata.t) => {
             Actions.{
               category: Some(snippet.prefix),
               name: snippet.description,
               command: () =>
                 Snippets(
                   Feature_Snippets.Msg.insert(~snippet=snippet.snippet),
                 ),
               icon: None,
               highlight: [],
               handle: None,
             }
           })
        |> Array.of_list;

      (
        Some({...Quickmenu.defaults(SnippetPicker(snippets)), items}),
        Isolinear.Effect.none,
      );

    | QuickmenuShow(SnippetFilePicker(snippetFiles)) =>
      let items =
        snippetFiles
        |> List.filter_map(
             (snippetFile: Service_Snippets.SnippetFileMetadata.t) => {
             Fp.baseName(snippetFile.filePath)
             |> Option.map(filePath => {
                  Actions.{
                    category:
                      Some(
                        snippetFile.language
                        |> Option.value(~default="global"),
                      ),
                    name:
                      snippetFile.isCreated
                        ? Printf.sprintf("Edit %s", filePath)
                        : Printf.sprintf("Create %s", filePath),
                    command: () =>
                      Snippets(
                        Feature_Snippets.Msg.editSnippetFile(~snippetFile),
                      ),
                    icon: None,
                    highlight: [],
                    handle: None,
                  }
                })
           })
        |> Array.of_list;

      (
        Some({
          ...Quickmenu.defaults(SnippetFilePicker(snippetFiles)),
          items,
        }),
        Isolinear.Effect.none,
      );

    | QuickmenuShow(OpenBuffersPicker) =>
      let items =
        makeBufferCommands(workspace, languageInfo, iconTheme, buffers);

      (
        Some({...Quickmenu.defaults(OpenBuffersPicker), items}),
        Isolinear.Effect.none,
      );

    | QuickmenuShow(FileTypesPicker({bufferId, languages})) =>
      if (Feature_Workspace.openedFolder(workspace) == None) {
        let items =
          makeBufferCommands(workspace, languageInfo, iconTheme, buffers);

        (
          Some({...Quickmenu.defaults(OpenBuffersPicker), items}),
          Isolinear.Effect.none,
        );
      } else {
        let items =
          languages
          |> List.map(((fileType, maybeIcon)) => {
               Actions.{
                 category: None,
                 name: fileType,
                 command: () =>
                   Buffers(
                     Feature_Buffers.Msg.fileTypeChanged(
                       ~bufferId,
                       ~fileType=Oni_Core.Buffer.FileType.explicit(fileType),
                     ),
                   ),
                 icon: maybeIcon,
                 highlight: [],
                 handle: None,
               }
             })
          |> Array.of_list;

        (
          Some({
            ...Quickmenu.defaults(FileTypesPicker({bufferId, languages})),
            items,
          }),
          Isolinear.Effect.none,
        );
      }

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

    | QuickmenuUpdateExtensionItems({items, _}) => (
        Option.map(
          (state: Quickmenu.t) => {
            switch (state.variant) {
            | Extension({id, resolver, _}) => {
                ...
                  Quickmenu.defaults(
                    Extension({id, hasItems: true, resolver}),
                  ),
                focused: None,
                items:
                  items |> List.map(Internal.extensionItem) |> Array.of_list,
              }
            | _ => state
            }
          },
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

    | ListSelect =>
      switch (state) {
      | Some({variant: Wildmenu(_), _}) => (None, executeVimCommandEffect)

      | Some({
          variant: Extension({resolver, _}),
          items,
          focused: Some(focused),
          _,
        }) =>
        switch (items[focused]) {
        | item => (None, selectExtensionItemEffect(~resolver, item))
        | exception (Invalid_argument(_)) => (
            None,
            cancelExtensionMenuEffect(~resolver),
          )
        }

      | Some({items, focused: Some(focused), _}) =>
        switch (items[focused]) {
        | item => (None, selectItemEffect(item))
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

  let updater = (state: State.t, action: Actions.t) => {
    let (menuState, menuEffect) =
      menuUpdater(
        state.quickmenu,
        action,
        state.buffers,
        state.languageInfo,
        state.iconTheme,
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
    QuickmenuFilterSubscription.create(
      ~id="quickmenu-filter",
      ~query,
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

  let ripgrep = (workspace, languageInfo, iconTheme, configuration) => {
    let filesExclude =
      Configuration.getValue(c => c.filesExclude, configuration);

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
          command: () => Actions.OpenFileByPath(fullPath, None, None),
          icon:
            Component_FileExplorer.getFileIcon(
              ~languageInfo,
              ~iconTheme,
              fullPath,
            ),
          highlight: [],
          handle: None,
        };
      [
        RipgrepSubscription.create(
          ~id="workspace-search",
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
      | ThemesPicker(_) => [filter(query, quickmenu.items)]
      | SnippetPicker(_) => [filter(query, quickmenu.items)]
      | SnippetFilePicker(_) => [filter(query, quickmenu.items)]

      | Extension({hasItems, _}) =>
        hasItems ? [filter(query, quickmenu.items)] : []

      | FilesPicker =>
        [filter(query, quickmenu.items)]
        @ ripgrep(
            state.workspace,
            state.languageInfo,
            state.iconTheme,
            state.configuration,
          )

      | FileTypesPicker(_) => [filter(query, quickmenu.items)]

      | Wildmenu(_) => []
      };

    | None => []
    };
  };

  updater;
};
