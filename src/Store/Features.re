open EditorCoreTypes;
open Isolinear;
open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Actions;

module Internal = {
  let notificationEffect = (~ephemeral=false, ~kind, message) => {
    Feature_Notification.Effects.create(~ephemeral, ~kind, message)
    |> Isolinear.Effect.map(msg => Actions.Notification(msg));
  };

  let openFileEffect =
      (~direction=SplitDirection.Current, ~position=None, filePath) => {
    Isolinear.Effect.createWithDispatch(
      ~name="features.openFileByPath", dispatch =>
      dispatch(OpenFileByPath(filePath, direction, position))
    );
  };

  let previewFileEffect = (~position=None, filePath) => {
    Isolinear.Effect.createWithDispatch(
      ~name="features.previewFileByPath", dispatch =>
      dispatch(PreviewFileByPath(filePath, SplitDirection.Current, position))
    );
  };

  let executeCommandEffect = (command, arguments) => {
    Isolinear.Effect.createWithDispatch(
      ~name="features.executeCommand", dispatch =>
      dispatch(Actions.KeybindingInvoked({command, arguments}))
    );
  };

  let showThemePicker = (themes: list(Feature_Theme.theme), state: State.t) => {
    open Exthost.Extension;
    let label = theme => Contributions.Theme.label(theme);
    let id = theme => Contributions.Theme.id(theme);

    let menu =
      Feature_Quickmenu.Schema.menu(
        ~onItemFocused=
          theme => {
            Actions.Theme(
              Feature_Theme.Msg.menuPreviewTheme(~themeId=id(theme)),
            )
          },
        ~onAccepted=
          (~text as _, ~item) => {
            item
            |> Option.map(item => {
                 Actions.Theme(
                   Feature_Theme.Msg.menuCommitTheme(~themeId=id(item)),
                 )
               })
            |> Option.value(~default=Actions.Noop)
          },
        ~toString=theme => "Theme: " ++ label(theme),
        themes,
      );
    {
      ...state,
      newQuickmenu: Feature_Quickmenu.show(~menu, state.newQuickmenu),
    };
  };

  let setThemes =
      (
        ~themes: list(Exthost.Extension.Contributions.Theme.t),
        state: State.t,
      ) => {
    let id = Exthost.Extension.Contributions.Theme.id;
    switch (themes) {
    | [] => state
    | [theme] => {
        ...state,
        colorTheme:
          Feature_Theme.setTheme(~themeId=id(theme), state.colorTheme),
      }
    | themes => showThemePicker(themes, state)
    };
  };

  let unhandledWindowMotionEffect = (windowMsg: Component_VimWindows.outmsg) =>
    Isolinear.Effect.createWithDispatch(
      ~name="features.unhandledWindowMotion", dispatch => {
      Component_VimWindows.(
        (
          switch (windowMsg) {
          | Nothing => Actions.Noop
          | FocusLeft => Actions.Layout(Feature_Layout.Msg.moveLeft)
          | FocusRight => Actions.Layout(Feature_Layout.Msg.moveRight)
          | FocusUp => Actions.Layout(Feature_Layout.Msg.moveUp)
          | FocusDown => Actions.Layout(Feature_Layout.Msg.moveDown)
          | PreviousTab => Actions.Noop
          | NextTab => Actions.Noop
          }
        )
        |> dispatch
      )
    });

  let getScopeForBuffer = (~languageInfo, buffer: Oni_Core.Buffer.t) => {
    buffer
    |> Oni_Core.Buffer.getFileType
    |> Oni_Core.Buffer.FileType.toString
    |> Exthost.LanguageInfo.getScopeFromLanguage(languageInfo)
    |> Option.value(~default="source.plaintext");
  };

  let quitEffect =
    Isolinear.Effect.createWithDispatch(~name="quit", dispatch =>
      dispatch(Actions.ReallyQuitting)
    );

  let chdir = (path: FpExp.t(FpExp.absolute)) => {
    Feature_Workspace.Effects.changeDirectory(path)
    |> Isolinear.Effect.map(msg => Actions.Workspace(msg));
  };

  let updateEditor = (~editorId, ~msg, ~client, layout) => {
    switch (Feature_Layout.editorById(editorId, layout)) {
    | Some(editor) =>
      open Feature_Editor;

      let (updatedEditor, outmsg) = update(editor, msg);
      let layout =
        Feature_Layout.map(
          editor => Editor.getId(editor) == editorId ? updatedEditor : editor,
          layout,
        );

      let effect =
        switch (outmsg) {
        | Nothing => Effect.none
        | MouseHovered(maybeCharacterPosition) =>
          Effect.createWithDispatch(~name="editor.mousehovered", dispatch => {
            dispatch(
              LanguageSupport(
                Feature_LanguageSupport.Msg.Hover.mouseHovered(
                  maybeCharacterPosition,
                ),
              ),
            )
          })
        | MouseMoved(maybeCharacterPosition) =>
          Effect.createWithDispatch(~name="editor.mousemoved", dispatch => {
            dispatch(
              LanguageSupport(
                Feature_LanguageSupport.Msg.Hover.mouseMoved(
                  maybeCharacterPosition,
                ),
              ),
            )
          })

        | ExecuteCommand(command) =>
          switch (command.id) {
          | None => Isolinear.Effect.none
          | Some(cmdId) =>
            Service_Exthost.Effects.Commands.executeContributedCommand(
              ~command=cmdId,
              ~arguments=command.arguments,
              client,
            )
          }
        | DisplayMenuAt({menu, xPos, yPos}) =>
          Feature_ContextMenu.Effects.displayMenuAt(
            ~menuSchema=menu,
            ~xPos,
            ~yPos,
          )
          |> Isolinear.Effect.map(msg => ContextMenu(msg))
        };

      (layout, effect);
    | None => (layout, Effect.none)
    };
  };

  let updateEditors =
      (
        ~scope: EditorScope.t,
        ~msg: Feature_Editor.msg,
        ~client: Exthost.Client.t,
        layout: Feature_Layout.model,
      ) => {
    switch (scope) {
    | All =>
      let (layout', effects) =
        Feature_Layout.fold(
          (prev, editor) => {
            let (layout, effects) = prev;
            let editorId = Feature_Editor.Editor.getId(editor);
            let (layout', effect') =
              updateEditor(~editorId, ~msg, ~client, layout);
            (layout', [effect', ...effects]);
          },
          (layout, []),
          layout,
        );
      (layout', Isolinear.Effect.batch(effects));
    | Editor(editorId) => updateEditor(~client, ~editorId, ~msg, layout)
    };
  };

  let updateConfiguration:
    State.t => (State.t, Isolinear.Effect.t(Actions.t)) =
    state => {
      let resolver = Selectors.configResolver(state);
      let maybeRoot = Feature_Explorer.root(state.fileExplorer);

      let fileExplorer =
        state.fileExplorer
        |> Feature_Explorer.configurationChanged(~config=resolver);
      let zen = Feature_Zen.configurationChanged(resolver, state.zen);
      let sideBar =
        state.sideBar
        |> Feature_SideBar.configurationChanged(
             ~hasWorkspace=maybeRoot != None,
             ~config=resolver,
           );

      let proxy = state.proxy |> Feature_Proxy.configurationChanged(resolver);
      let colorTheme =
        state.colorTheme |> Feature_Theme.configurationChanged(~resolver);

      let buffers =
        state.buffers
        |> Feature_Buffers.configurationChanged(~config=resolver);

      let languageSupport =
        state.languageSupport
        |> Feature_LanguageSupport.configurationChanged(~config=resolver);

      let perFileTypeConfig =
        Feature_Configuration.resolver(state.config, state.vim);

      let layout =
        Feature_Layout.map(
          editor => {
            Feature_Editor.Editor.configurationChanged(
              ~perFileTypeConfig,
              editor,
            )
          },
          state.layout,
        );

      let terminals =
        Feature_Terminal.configurationChanged(
          ~config=resolver,
          state.terminals,
        );

      let vim = Feature_Vim.configurationChanged(~config=resolver, state.vim);
      let vimEffect =
        Oni_Core.EffectEx.value(
          ~name="Feature_vim.sychronizeExperimentalViml",
          Oni_Model.Actions.SynchronizeExperimentalViml(
            Feature_Vim.experimentalViml(vim),
          ),
        );

      let (zoom, zoomEffect) =
        Feature_Zoom.configurationChanged(~config=resolver, state.zoom);
      let eff = zoomEffect |> Isolinear.Effect.map(msg => Actions.Zoom(msg));
      (
        {
          ...state,
          buffers,
          colorTheme,
          fileExplorer,
          languageSupport,
          sideBar,
          layout,
          proxy,
          terminals,
          vim,
          zen,
          zoom,
        },
        Isolinear.Effect.batch([eff, vimEffect]),
      );
    };

  let updateMode =
      (
        ~client: Exthost.Client.t,
        ~allowAnimation,
        state: State.t,
        mode: Vim.Mode.t,
        effects,
      ) => {
    let prevCursor =
      state.layout
      |> Feature_Layout.activeEditor
      |> Feature_Editor.Editor.getPrimaryCursor;

    let config = Selectors.configResolver(state);
    let editor = Feature_Layout.activeEditor(state.layout);
    let maybeBuffer = Selectors.getActiveBuffer(state);

    let wasInInsertMode =
      Vim.Mode.isInsertOrSelect(
        state.layout
        |> Feature_Layout.activeEditor
        |> Feature_Editor.Editor.mode,
      );

    let activeEditorId = editor |> Feature_Editor.Editor.getId;

    let msg: Feature_Editor.msg =
      ModeChanged({allowAnimation, mode, effects});
    let scope = EditorScope.Editor(activeEditorId);
    let (layout, editorEffect) =
      updateEditors(~client, ~scope, ~msg, state.layout);

    let isInInsertMode =
      Vim.Mode.isInsertOrSelect(
        layout |> Feature_Layout.activeEditor |> Feature_Editor.Editor.mode,
      );

    let newCursor =
      layout
      |> Feature_Layout.activeEditor
      |> Feature_Editor.Editor.getPrimaryCursor;

    let languageInfo =
      state.languageSupport |> Feature_LanguageSupport.languageInfo;
    let languageSupport =
      if (prevCursor != newCursor) {
        maybeBuffer
        |> Option.map(buffer => {
             let languageConfiguration =
               buffer
               |> Oni_Core.Buffer.getFileType
               |> Oni_Core.Buffer.FileType.toString
               |> Exthost.LanguageInfo.getLanguageConfiguration(languageInfo)
               |> Option.value(~default=LanguageConfiguration.default);
             Feature_LanguageSupport.cursorMoved(
               ~editorId=activeEditorId,
               ~languageConfiguration,
               ~buffer,
               ~previous=prevCursor,
               ~current=newCursor,
               state.languageSupport,
             );
           })
        |> Option.value(~default=state.languageSupport);
      } else {
        state.languageSupport;
      };

    let wasInSnippetMode = Feature_Snippets.isActive(state.snippets);
    let snippets' = Feature_Snippets.modeChanged(~mode, state.snippets);
    let isInSnippetMode = Feature_Snippets.isActive(snippets');

    let languageSupport' =
      if (isInInsertMode != wasInInsertMode) {
        if (isInInsertMode) {
          languageSupport
          |> Feature_LanguageSupport.startInsertMode(~config, ~maybeBuffer);
        } else {
          languageSupport |> Feature_LanguageSupport.stopInsertMode;
        };
      } else {
        languageSupport;
      };

    let languageSupport'' =
      if (wasInSnippetMode != isInSnippetMode) {
        if (isInSnippetMode) {
          Feature_LanguageSupport.startSnippet(languageSupport');
        } else {
          Feature_LanguageSupport.stopSnippet(languageSupport');
        };
      } else {
        languageSupport';
      };

    let state = {
      ...state,
      layout,
      languageSupport: languageSupport'',
      snippets: snippets',
    };
    (state, editorEffect);
  };
};

// UPDATE

let update =
    (
      ~grammarRepository: Oni_Syntax.GrammarRepository.t,
      ~extHostClient: Exthost.Client.t,
      ~maximize,
      ~minimize,
      ~close,
      ~restore,
      ~window,
      ~setVsync,
      state: State.t,
      action: Actions.t,
    ) =>
  switch (action) {
  | ClientServer(msg) =>
    let (clientServer, outmsg) =
      Feature_ClientServer.update(msg, state.clientServer);

    let eff =
      switch (outmsg) {
      | Nothing => Isolinear.Effect.none
      | OpenFilesAndFolders({files, folder}) =>
        let folderEffect =
          folder
          |> Option.map(Internal.chdir)
          |> Option.value(~default=Isolinear.Effect.none);

        let openFileEffects =
          files
          |> List.map(FpExp.toString)
          |> List.map(
               Internal.openFileEffect(~direction=SplitDirection.Inactive),
             );

        [folderEffect, ...openFileEffects] |> Isolinear.Effect.batch;
      };

    ({...state, clientServer}, eff);
  | Clipboard(msg) =>
    let (model, outmsg) = Feature_Clipboard.update(msg, state.clipboard);

    let eff =
      switch (outmsg) {
      | Nothing => Isolinear.Effect.none
      | Effect(eff) =>
        eff |> Isolinear.Effect.map(msg => Actions.Clipboard(msg))
      | Pasted({rawText, isMultiLine, lines}) =>
        Isolinear.Effect.createWithDispatch(~name="Clipboard.Pasted", dispatch => {
          dispatch(Actions.Pasted({rawText, isMultiLine, lines}))
        })
      };

    ({...state, clipboard: model}, eff);

  | Decorations(msg) =>
    let (model, outMsg) =
      Feature_Decorations.update(
        ~client=extHostClient,
        msg,
        state.decorations,
      );

    let state = {...state, decorations: model};
    let eff =
      switch (outMsg) {
      | Feature_Decorations.Nothing => Isolinear.Effect.none
      | Feature_Decorations.Effect(eff) =>
        eff |> Isolinear.Effect.map(msg => Decorations(msg))
      };
    (state, eff);

  | Diagnostics(msg) =>
    let config = Selectors.configResolver(state);
    let previewEnabled =
      Feature_Configuration.GlobalConfiguration.Workbench.editorEnablePreview.
        get(
        config,
      );
    let (diagnostics, outmsg) =
      Feature_Diagnostics.update(~previewEnabled, msg, state.diagnostics);

    let state' = {...state, diagnostics};
    switch (outmsg) {
    | Nothing => (state', Isolinear.Effect.none)
    | OpenFile({filePath, position}) => (
        state',
        Internal.openFileEffect(~position=Some(position), filePath),
      )
    | PreviewFile({filePath, position}) => (
        state',
        Internal.previewFileEffect(~position=Some(position), filePath),
      )

    | TogglePane({paneId}) => (
        state',
        EffectEx.value(
          ~name="Feature_Diagnostics.Effect.togglePane",
          Actions.Pane(Feature_Pane.Msg.toggle(~paneId)),
        ),
      )
    };

  | Exthost(msg) =>
    let (model, outMsg) =
      Feature_Exthost.update(
        ~buffers=state.buffers,
        ~editors=Feature_Layout.visibleEditors(state.layout),
        msg,
        state.exthost,
      );

    let state = {...state, exthost: model};
    let eff =
      switch (outMsg) {
      | Feature_Exthost.Nothing => Isolinear.Effect.none
      | Feature_Exthost.Effect(eff) =>
        eff |> Isolinear.Effect.map(msg => Exthost(msg))
      };
    (state, eff);

  | Extensions(msg) =>
    let (model, outMsg) =
      Feature_Extensions.update(
        ~extHostClient,
        ~proxy=state.proxy |> Feature_Proxy.proxy,
        msg,
        state.extensions,
      );
    let state = {...state, extensions: model};
    let (state', effect) =
      Feature_Extensions.(
        switch (outMsg) {
        | Nothing => (state, Effect.none)
        | Effect(eff) => (
            state,
            eff |> Isolinear.Effect.map(msg => Actions.Extensions(msg)),
          )
        | Focus => (FocusManager.push(Focus.Extensions, state), Effect.none)
        | NotifySuccess(msg) => (
            state,
            Internal.notificationEffect(~kind=Info, msg),
          )
        | NotifyFailure(msg) => (
            state,
            Internal.notificationEffect(~kind=Error, msg),
          )

        | OpenExtensionDetails =>
          let eff = Internal.openFileEffect("oni://ExtensionDetails");
          (state, eff);

        | SelectTheme({themes}) => (
            state |> Internal.setThemes(~themes),
            Isolinear.Effect.none,
          )

        | UnhandledWindowMovement(movement) => (
            state,
            Internal.unhandledWindowMotionEffect(movement),
          )

        | NewExtensions(extensions) =>
          let newExtensionConfigurations =
            extensions
            |> List.map((ext: Exthost.Extension.Scanner.ScanResult.t) => {
                 Exthost.Extension.Manifest.(
                   ext.manifest.contributes.configuration
                 )
               });

          let languageSupport' =
            Feature_LanguageSupport.extensionsAdded(
              extensions,
              state.languageSupport,
            );
          let config =
            Feature_Configuration.registerExtensionConfigurations(
              ~configurations=newExtensionConfigurations,
              state.config,
            );

          (
            {...state, config, languageSupport: languageSupport'},
            Isolinear.Effect.none,
          );

        | InstallSucceeded({extensionId, contributions}) =>
          let notificationEffect =
            Internal.notificationEffect(
              ~kind=Info,
              Printf.sprintf(
                "Extension %s was installed successfully and will be activated on restart.",
                extensionId,
              ),
            );
          let themes: list(Exthost.Extension.Contributions.Theme.t) =
            Exthost.Extension.Contributions.(contributions.themes);
          (state |> Internal.setThemes(~themes), notificationEffect);
        }
      );
    (state', effect);

  | FileExplorer(msg) =>
    let config = Selectors.configResolver(state);
    let (model, outmsg) =
      Feature_Explorer.update(~config, msg, state.fileExplorer);

    let state = {...state, fileExplorer: model};

    Feature_Explorer.(
      switch (outmsg) {
      | Nothing => (state, Isolinear.Effect.none)
      | Effect(effect) => (
          state,
          effect |> Isolinear.Effect.map(msg => FileExplorer(msg)),
        )
      | OpenFile(filePath) => (state, Internal.openFileEffect(filePath))
      | PreviewFile(filePath) => (
          state,
          Internal.previewFileEffect(filePath),
        )
      | GrabFocus => (
          FocusManager.push(Focus.FileExplorer, state),
          Isolinear.Effect.none,
        )
      | PickFolder => (
          state,
          Feature_Workspace.Effects.pickFolder
          |> Isolinear.Effect.map(msg => Workspace(msg)),
        )
      | UnhandledWindowMovement(windowMovement) => (
          state,
          Internal.unhandledWindowMotionEffect(windowMovement),
        )
      | SymbolSelected(symbol) =>
        let maybeBuffer = Oni_Model.Selectors.getActiveBuffer(state);
        let eff =
          maybeBuffer
          |> OptionEx.flatMap(Oni_Core.Buffer.getFilePath)
          |> Option.map(filePath => {
               let range: CharacterRange.t =
                 Feature_LanguageSupport.DocumentSymbols.(symbol.range);
               let position = range.start;

               Internal.openFileEffect(~position=Some(position), filePath);
             })
          |> Option.value(~default=Isolinear.Effect.none);
        (state, eff);

      | WatchedPathChanged({path, stat}) =>
        let eff =
          Service_Exthost.Effects.FileSystemEventService.onPathChanged(
            ~path,
            ~maybeStat=stat,
            extHostClient,
          );
        (state, eff);
      }
    );

  | ContextMenu(msg) =>
    let contextKeys = Oni_Model.ContextKeys.all(state);
    let commands = CommandManager.current(state);
    let config = Selectors.configResolver(state);
    let (model, outmsg) =
      Feature_ContextMenu.update(
        ~contextKeys,
        ~commands,
        ~config,
        ~input=state.input,
        ~window,
        msg,
        state.contextMenu,
      );

    let eff =
      Feature_ContextMenu.(
        switch (outmsg) {
        | Nothing => Isolinear.Effect.none
        | ExecuteCommand({command}) =>
          Internal.executeCommandEffect(command, `Null)
        | Effect(eff) =>
          eff |> Isolinear.Effect.map(msg => Actions.ContextMenu(msg))
        }
      );

    ({...state, contextMenu: model}, eff);

  | FileSystem(msg) =>
    let (model, outmsg) = Feature_FileSystem.update(msg, state.fileSystem);

    let eff =
      switch (outmsg) {
      | Nothing => Isolinear.Effect.none
      | Effect(eff) => eff |> Isolinear.Effect.map(msg => FileSystem(msg))
      };
    ({...state, fileSystem: model}, eff);

  | Help(msg) =>
    let (model, outmsg) = Feature_Help.update(msg, state.help);
    let eff =
      switch (outmsg) {
      | Nothing => Isolinear.Effect.none
      | Effect(effect) => effect |> Isolinear.Effect.map(msg => Help(msg))
      };
    ({...state, help: model}, eff);

  | Input(msg) =>
    let (model, outmsg) = Feature_Input.update(msg, state.input);

    let eff =
      switch (outmsg) {
      | Nothing => Isolinear.Effect.none
      | DebugInputShown => Internal.openFileEffect("oni://DebugInput")

      | ErrorNotifications(errorMessages) =>
        errorMessages
        |> List.map(msg => Internal.notificationEffect(~kind=Error, msg))
        |> Isolinear.Effect.batch

      | MapParseError({fromKeys, toKeys, error}) =>
        Internal.notificationEffect(
          ~kind=Error,
          Printf.sprintf(
            "Error mapping %s to %s: %s",
            fromKeys,
            toKeys,
            error,
          ),
        )

      | OpenFile(fp) => Internal.openFileEffect(FpExp.toString(fp))

      | TimedOut =>
        Isolinear.Effect.createWithDispatch(~name="Input.timeout", dispatch =>
          dispatch(KeyTimeout)
        )
      };

    ({...state, input: model}, eff);

  | Keyboard(msg) =>
    let (model, outmsg) = Feature_Keyboard.update(state.keyboard, msg);
    switch (outmsg) {
    | Nothing => ({...state, keyboard: model}, Isolinear.Effect.none)
    | Effect(eff) => (
        {...state, keyboard: model},
        eff |> Isolinear.Effect.map(eff => Keyboard(eff)),
      )
    };

  | LanguageSupport(msg) =>
    let maybeBuffer = Oni_Model.Selectors.getActiveBuffer(state);
    let editor = state.layout |> Feature_Layout.activeEditor;
    let cursorLocation = editor |> Feature_Editor.Editor.getPrimaryCursor;

    let selection = editor |> Feature_Editor.Editor.selectionOrCursorRange;

    let editorId = editor |> Feature_Editor.Editor.getId;

    let languageInfo =
      state.languageSupport |> Feature_LanguageSupport.languageInfo;
    let languageConfiguration =
      maybeBuffer
      |> Option.map(Oni_Core.Buffer.getFileType)
      |> Option.map(Oni_Core.Buffer.FileType.toString)
      |> OptionEx.flatMap(
           Exthost.LanguageInfo.getLanguageConfiguration(languageInfo),
         )
      |> Option.value(~default=LanguageConfiguration.default);

    let characterSelection =
      editor |> Feature_Editor.Editor.byteRangeToCharacterRange(selection);

    let config = Selectors.configResolver(state);
    let previewEnabled =
      Feature_Configuration.GlobalConfiguration.Workbench.editorEnablePreview.
        get(
        config,
      );

    let (languageSupport, outmsg) =
      Feature_LanguageSupport.update(
        ~buffers=state.buffers,
        ~config,
        ~diagnostics=state.diagnostics,
        ~languageInfo,
        ~font=state.editorFont,
        ~languageConfiguration,
        ~previewEnabled,
        ~extensions=state.extensions,
        ~maybeBuffer,
        ~maybeSelection=characterSelection,
        ~editorId,
        ~cursorLocation,
        ~client=extHostClient,
        msg,
        state.languageSupport,
      );

    let state = {...state, languageSupport};

    let exthostEditToVimEdit: Exthost.Edit.SingleEditOperation.t => Vim.Edit.t = (
      exthostEdit => {
        let range = exthostEdit.range |> Exthost.OneBasedRange.toRange;

        let text =
          exthostEdit.text
          |> Option.map(str => {
               str |> String.split_on_char('\n') |> Array.of_list
             })
          |> Option.value(~default=[||]);

        Vim.Edit.{range, text};
      }
    );

    Feature_LanguageSupport.(
      switch (outmsg) {
      | Nothing => (state, Isolinear.Effect.none)
      | ApplyCompletion({insertText, replaceSpan, additionalEdits}) =>
        let additionalEdits =
          additionalEdits |> List.map(exthostEditToVimEdit);
        (
          state,
          Feature_Vim.Effects.applyCompletion(
            ~cursor=cursorLocation,
            ~additionalEdits,
            ~replaceSpan,
            ~insertText,
          )
          |> Isolinear.Effect.map(msg => Vim(msg)),
        );

      | ApplyWorkspaceEdit(workspaceEdit) =>
        let toVimEdits = (~buffer, workspaceEdit: Exthost.WorkspaceEdit.t) => {
          workspaceEdit.edits
          |> List.filter_map(
               fun
               // TODO: Handle workspace edits
               | Exthost.WorkspaceEdit.File(_) => None
               | Exthost.WorkspaceEdit.Text(edit) => Some(edit),
             )
          |> List.filter((textEdit: Exthost.WorkspaceEdit.TextEdit.t) => {
               let bufferUri = Buffer.getUri(buffer);
               Uri.equals(bufferUri, textEdit.resource);
             })
          |> List.map((textEdit: Exthost.WorkspaceEdit.TextEdit.t) => {
               let edit = textEdit.edit;
               let text =
                 edit.text
                 |> Oni_Core.Utility.StringEx.removeWindowsNewLines
                 |> Oni_Core.Utility.StringEx.splitNewLines;

               Vim.Edit.{
                 range: edit.range |> Exthost.OneBasedRange.toRange,
                 text,
               };
             });
        };
        let eff =
          switch (maybeBuffer) {
          | Some(buffer) =>
            let edits = toVimEdits(~buffer, workspaceEdit);

            Service_Vim.Effects.applyEdits(
              ~shouldAdjustCursors=true,
              ~bufferId=buffer |> Oni_Core.Buffer.getId,
              ~version=buffer |> Oni_Core.Buffer.getVersion,
              ~edits,
              fun
              | _ => Noop,
            );
          | None => Isolinear.Effect.none
          };
        (state, eff);

      | FormattingApplied({displayName, editCount, needsToSave}) =>
        let (formatEffect, msg) =
          if (needsToSave) {
            state
            |> Selectors.getActiveBuffer
            |> Option.map(Oni_Core.Buffer.getId)
            |> Option.map(bufferId => {
                 (
                   Feature_Vim.Effects.save(~bufferId)
                   |> Isolinear.Effect.map(msg => Actions.Vim(msg)),
                   Printf.sprintf(
                     "Formatting: Saved %d edits with %s",
                     editCount,
                     displayName,
                   ),
                 )
               })
            |> Option.value(
                 ~default=(
                   Isolinear.Effect.none,
                   "Unable to apply formatting edits",
                 ),
               );
          } else {
            (
              Isolinear.Effect.none,
              Printf.sprintf(
                "Formatting: Applied %d edits with %s",
                editCount,
                displayName,
              ),
            );
          };

        let eff =
          [
            formatEffect,
            Internal.notificationEffect(~ephemeral=true, ~kind=Info, msg),
          ]
          |> Isolinear.Effect.batch;
        (state, eff);
      | ReferencesAvailable =>
        let effect =
          EffectEx.value(
            ~name="Feature_LanguageSupport.References.Effect.togglePane",
            Actions.Pane(
              Feature_Pane.Msg.toggle(~paneId="workbench.panel.locations"),
            ),
          );
        (state, effect);
      | InsertSnippet({replaceSpan, snippet, _}) =>
        let editor = Feature_Layout.activeEditor(state.layout);
        // let cursor = Feature_Editor.Editor.getPrimaryCursor(editor);
        // let characterPosition =
        //   CharacterPosition.{line: cursor.line, character: meetColumn};
        // let rangeToReplace =
        //   CharacterRange.{start: characterPosition, stop: cursor};
        let maybeReplaceRange =
          Feature_Editor.Editor.characterRangeToByteRange(
            EditorCoreTypes.CharacterSpan.toRange(
              ~line=cursorLocation.line,
              replaceSpan,
            ),
            editor,
          );
        (
          state,
          Feature_Snippets.Effects.insertSnippet(
            ~replaceRange=maybeReplaceRange,
            ~snippet,
          )
          |> Isolinear.Effect.map(msg => Snippets(msg)),
        );
      | PreviewFile({filePath, position}) => (
          state,
          Internal.previewFileEffect(~position=Some(position), filePath),
        )
      | OpenFile({filePath, location, direction}) => (
          state,
          Internal.openFileEffect(~direction, ~position=location, filePath),
        )
      | NotifySuccess(msg) => (
          state,
          Internal.notificationEffect(~kind=Info, msg),
        )
      | NotifyFailure(msg) => (
          state,
          Internal.notificationEffect(~kind=Error, msg),
        )
      | Effect(eff) => (
          state,
          eff |> Isolinear.Effect.map(msg => LanguageSupport(msg)),
        )
      | CodeLensesChanged({handle, bufferId, startLine, stopLine, lenses}) =>
        let layout' =
          state.layout
          |> Feature_Layout.map(editor =>
               if (Feature_Editor.Editor.getBufferId(editor) == bufferId) {
                 Feature_Editor.Editor.setCodeLens(
                   ~uiFont=state.uiFont,
                   ~startLine,
                   ~stopLine,
                   ~handle,
                   ~lenses,
                   editor,
                 );
               } else {
                 editor;
               }
             );
        ({...state, layout: layout'}, Isolinear.Effect.none);

      | SetSelections({editorId, ranges}) =>
        let layout' =
          state.layout
          |> Feature_Layout.map(editor =>
               Feature_Editor.(
                 if (Editor.getId(editor) == editorId) {
                   let byteRanges =
                     ranges
                     |> List.filter_map(characterRange =>
                          Editor.characterRangeToByteRange(
                            characterRange,
                            editor,
                          )
                        );

                   Editor.setSelections(byteRanges, editor);
                 } else {
                   editor;
                 }
               )
             );
        ({...state, layout: layout'}, Isolinear.Effect.none);

      | ShowMenu(menu) =>
        let menu' =
          menu
          |> Feature_Quickmenu.Schema.map(msg => Actions.LanguageSupport(msg));
        let quickmenu' =
          Feature_Quickmenu.show(~menu=menu', state.newQuickmenu);
        ({...state, newQuickmenu: quickmenu'}, Isolinear.Effect.none);

      | TransformConfiguration(transformer) =>
        let config' =
          Feature_Configuration.queueTransform(~transformer, state.config);
        ({...state, config: config'}, Isolinear.Effect.none);
      }
    );

  | Logging(msg) =>
    let (model, outmsg) = Feature_Logging.update(msg, state.logging);
    let state' = {...state, logging: model};

    let eff =
      Feature_Logging.(
        {
          switch (outmsg) {
          | Effect(eff) =>
            eff |> Isolinear.Effect.map(msg => Actions.Logging(msg))
          | ShowInfoNotification(msg) =>
            Internal.notificationEffect(~kind=Info, msg)
          };
        }
      );
    (state', eff);

  | MenuBar(msg) =>
    let contextKeys = Oni_Model.ContextKeys.all(state);
    let commands = CommandManager.current(state);
    let (menuBar', outmsg) =
      Feature_MenuBar.update(~contextKeys, ~commands, msg, state.menuBar);

    let eff =
      switch (outmsg) {
      | Nothing => Isolinear.Effect.none
      | ExecuteCommand({command}) =>
        Internal.executeCommandEffect(command, `Null)
      };
    ({...state, menuBar: menuBar'}, eff);

  | Messages(msg) =>
    let (model, outmsg) = Feature_Messages.update(msg, state.messages);
    let state = {...state, messages: model};

    let eff =
      Feature_Messages.(
        switch (outmsg) {
        | Nothing => Isolinear.Effect.none
        | Notification({kind, message}) =>
          Internal.notificationEffect(~kind, message)
        | Effect(eff) =>
          eff |> Isolinear.Effect.map(msg => Actions.Messages(msg))
        }
      );
    (state, eff);

  | Pane(msg) =>
    let (model, outmsg) = Feature_Pane.update(msg, state.pane);

    let state = {...state, pane: model};

    switch (outmsg) {
    | Nothing => (state, Isolinear.Effect.none)
    | UnhandledWindowMovement(windowMovement) => (
        state,
        Internal.unhandledWindowMotionEffect(windowMovement),
      )
    | GrabFocus => (
        state |> FocusManager.push(Focus.Pane),
        Isolinear.Effect.none,
      )
    | ReleaseFocus => (
        state |> FocusManager.pop(Focus.Pane),
        Isolinear.Effect.none,
      )

    | NestedMessage(msg) => (
        state,
        EffectEx.value(~name="Feature_Pane.NestedMessage", msg),
      )
    // | PaneButton(pane) =>
    //   switch (pane) {
    //   | Notifications => (
    //       state,
    //       Feature_Notification.Effects.clear()
    //       |> Isolinear.Effect.map(msg => Notification(msg)),
    //     )
    //   | _ => (state, Isolinear.Effect.none)
    //   }
    };

  | Registers(msg) =>
    let (model, outmsg) = Feature_Registers.update(msg, state.registers);

    let state = {...state, registers: model};
    let eff =
      switch (outmsg) {
      | Feature_Registers.EmitRegister({raw, lines, _}) =>
        Isolinear.Effect.createWithDispatch(~name="register.paste", dispatch => {
          dispatch(
            Pasted({
              rawText: raw,
              isMultiLine: String.contains(raw, '\n'),
              lines,
            }),
          )
        })
      | Feature_Registers.FailedToGetRegister(c) =>
        Internal.notificationEffect(
          ~kind=Error,
          Printf.sprintf("No value at register %c", c),
        )
      | Effect(eff) =>
        eff |> Isolinear.Effect.map(msg => Actions.Registers(msg))
      | Nothing => Isolinear.Effect.none
      };
    (state, eff);

  | Registration(msg) =>
    let (state', outmsg) =
      Feature_Registration.update(
        ~proxy=state.proxy |> Feature_Proxy.proxy,
        state.registration,
        msg,
      );

    let effect =
      switch (outmsg) {
      | Nothing => Isolinear.Effect.none
      | Effect(eff) =>
        eff |> Isolinear.Effect.map(msg => Actions.Registration(msg))
      | LicenseKeyChanged(licenseKey) =>
        Service_AutoUpdate.Effect.updateLicenseKey(~licenseKey)
        |> Isolinear.Effect.map(msg =>
             Actions.AutoUpdate(Feature_AutoUpdate.Service(msg))
           )
      };
    ({...state, registration: state'}, effect);

  | Search(msg) =>
    let config = Selectors.configResolver(state);
    let (model, maybeOutmsg) =
      Feature_Search.update(
        ~previewEnabled=
          Feature_Configuration.GlobalConfiguration.Workbench.editorEnablePreview.
            get(
            config,
          ),
        state.searchPane,
        msg,
      );
    let state = {...state, searchPane: model};

    switch (maybeOutmsg) {
    | Some(Feature_Search.Focus) => (
        FocusManager.push(Focus.Search, state),
        Effect.none,
      )

    | Some(OpenFile({filePath, location})) => (
        state,
        Internal.openFileEffect(~position=Some(location), filePath),
      )

    | Some(PreviewFile({filePath, location})) => (
        state,
        Internal.previewFileEffect(~position=Some(location), filePath),
      )
    | Some(UnhandledWindowMovement(windowMovement)) => (
        state,
        Internal.unhandledWindowMotionEffect(windowMovement),
      )
    | None => (state, Effect.none)
    };

  | SCM(msg) =>
    let config = Selectors.configResolver(state);
    let (model, maybeOutmsg) =
      Feature_SCM.update(
        ~previewEnabled=
          Feature_Configuration.GlobalConfiguration.Workbench.editorEnablePreview.
            get(
            config,
          ),
        ~fileSystem=state.fileSystem,
        extHostClient,
        state.scm,
        msg,
      );
    let state = {...state, scm: model};

    switch ((maybeOutmsg: Feature_SCM.outmsg)) {
    | Focus => (FocusManager.push(Focus.SCM, state), Effect.none)
    | Effect(eff) => (state, eff |> Effect.map(msg => Actions.SCM(msg)))
    | EffectAndFocus(eff) => (
        FocusManager.push(Focus.SCM, state),
        eff |> Effect.map(msg => Actions.SCM(msg)),
      )

    | OpenFile(filePath) => (state, Internal.openFileEffect(filePath))
    | PreviewFile(filePath) => (state, Internal.previewFileEffect(filePath))

    | OriginalContentLoaded({bufferId, originalLines}) => (
        {
          ...state,
          buffers:
            Feature_Buffers.setOriginalLines(
              ~bufferId,
              ~originalLines,
              state.buffers,
            ),
        },
        Isolinear.Effect.none,
      )

    | UnhandledWindowMovement(windowMovement) => (
        state,
        Internal.unhandledWindowMotionEffect(windowMovement),
      )
    | Nothing => (state, Effect.none)
    };

  | SideBar(msg) =>
    let isFocused =
      FocusManager.current(state) == Focus.SCM
      || FocusManager.current(state) == Focus.Search
      || FocusManager.current(state) == Focus.FileExplorer
      || FocusManager.current(state) == Focus.Extensions;
    let (sideBar', outmsg) =
      Feature_SideBar.update(~isFocused, msg, state.sideBar);
    let state = {...state, sideBar: sideBar'};

    switch (outmsg) {
    | Nothing => (state, Effect.none)
    | PopFocus => (state |> FocusManager.push(Editor), Effect.none)
    | Focus(maybeSubFocus) =>
      let state' =
        Feature_SideBar.(
          switch (sideBar' |> Feature_SideBar.selected) {
          | FileExplorer =>
            let fileExplorer =
              switch (maybeSubFocus) {
              | Some(Outline) =>
                state.fileExplorer |> Feature_Explorer.focusOutline
              | None => state.fileExplorer
              };
            {...state, fileExplorer} |> FocusManager.push(Focus.FileExplorer);
          | SCM =>
            {...state, scm: Feature_SCM.resetFocus(state.scm)}
            |> FocusManager.push(Focus.SCM)
          | Search =>
            let query =
              FocusManager.current(state) == Focus.Editor
                ? state.layout
                  |> Feature_Layout.activeEditor
                  |> Feature_Editor.Editor.singleLineSelectedText
                : None;
            {
              ...state,
              searchPane: Feature_Search.resetFocus(~query, state.searchPane),
            }
            |> FocusManager.push(Focus.Search);
          | Extensions =>
            {
              ...state,
              extensions: Feature_Extensions.resetFocus(state.extensions),
            }
            |> FocusManager.push(Focus.Extensions)
          }
        );
      (
        {
          ...state',
          // When the sidebar acquires focus, zen-mode should be disabled
          zen: Feature_Zen.exitZenMode(state.zen),
        },
        Effect.none,
      );
    };

  | Sneak(msg) =>
    let (model, maybeOutmsg) = Feature_Sneak.update(state.sneak, msg);

    let state = {...state, sneak: model};

    let eff =
      switch ((maybeOutmsg: Feature_Sneak.outmsg)) {
      | Nothing => Effect.none
      | Effect(eff) => eff |> Effect.map(msg => Actions.Sneak(msg))
      };
    (state, eff);

  | StatusBar(msg) =>
    open Feature_StatusBar;
    let (statusBar', maybeOutmsg) =
      Feature_StatusBar.update(~client=extHostClient, state.statusBar, msg);

    let state' = {...state, statusBar: statusBar'};

    let (state'', eff) =
      switch ((maybeOutmsg: Feature_StatusBar.outmsg)) {
      | Nothing => (state', Effect.none)
      | ToggleProblems => (
          state',
          Feature_Pane.Msg.toggle(~paneId="workbench.panel.markers")
          |> EffectEx.value(~name="Feature_StatusBar.toggleProblems")
          |> Effect.map(msg => Pane(msg)),
        )

      | ClearNotifications => (
          {...state', notifications: Feature_Notification.initial},
          Effect.none,
        )
      | ToggleNotifications => (
          state,
          Feature_Pane.Msg.toggle(~paneId="workbench.panel.notifications")
          |> EffectEx.value(~name="Feature_StatusBar.toggleNotifications")
          |> Effect.map(msg => Pane(msg)),
        )
      | ShowFileTypePicker =>
        let bufferId =
          state.layout
          |> Feature_Layout.activeEditor
          |> Feature_Editor.Editor.getBufferId;

        (
          state',
          Isolinear.Effect.createWithDispatch(
            ~name="statusBar.fileTypePicker", dispatch => {
            dispatch(
              Actions.Buffers(
                Feature_Buffers.Msg.selectFileTypeClicked(~bufferId),
              ),
            )
          }),
        );

      | ShowIndentationPicker => (
          state',
          EffectEx.value(
            ~name="StatusBar.showIndentationPicker",
            Actions.Buffers(Feature_Buffers.Msg.statusBarIndentationClicked),
          ),
        )

      | Effect(eff) => (
          state',
          eff |> Isolinear.Effect.map(msg => Actions.StatusBar(msg)),
        )
      };

    (state'', eff);

  | Buffers(msg) =>
    let activeBufferId =
      state.layout
      |> Feature_Layout.activeEditor
      |> Feature_Editor.Editor.getBufferId;
    let config = Feature_Configuration.resolver(state.config, state.vim);

    let languageInfo =
      state.languageSupport |> Feature_LanguageSupport.languageInfo;
    let (buffers, outmsg) =
      Feature_Buffers.update(
        ~activeBufferId,
        ~config,
        ~languageInfo,
        ~workspace=state.workspace,
        msg,
        state.buffers,
      );

    let state = {...state, buffers};

    switch (outmsg) {
    | Nothing => (state, Effect.none)

    | Effect(eff) => (
        state,
        eff |> Isolinear.Effect.map(msg => Buffers(msg)),
      )

    | SetClipboardText(text) => (
        state,
        Service_Clipboard.Effects.setClipboardText(text),
      )

    | ShowMenu(menuFn) =>
      let languageInfo =
        state.languageSupport |> Feature_LanguageSupport.languageInfo;
      let menu =
        menuFn(languageInfo, state.iconTheme)
        |> Feature_Quickmenu.Schema.map(msg => Buffers(msg));
      let quickmenu = Feature_Quickmenu.show(~menu, state.newQuickmenu);
      ({...state, newQuickmenu: quickmenu}, Isolinear.Effect.none);

    | NotifyInfo(msg) => (
        state,
        Internal.notificationEffect(~kind=Info, msg),
      )

    | NotifyError(msg) => (
        state,
        Internal.notificationEffect(~kind=Error, msg),
      )

    | BufferModifiedSet(id, _) =>
      open Feature_Editor;

      let bufferId = id;

      let newLayout =
        Feature_Layout.map(
          editor =>
            if (Editor.getBufferId(editor) == bufferId
                && Editor.getPreview(editor) == true) {
              Editor.setPreview(~preview=false, editor);
            } else {
              editor;
            },
          state.layout,
        );

      ({...state, layout: newLayout}, Effect.none);

    | CreateEditor({buffer, split, position, grabFocus, preview}) =>
      open Feature_Editor;

      let editorBuffer = buffer |> EditorBuffer.ofBuffer;
      let fileType = buffer |> Buffer.getFileType |> Buffer.FileType.toString;

      let bufferId = EditorBuffer.id(editorBuffer);

      let existingEditor =
        Feature_Layout.activeGroupEditors(state.layout)
        |> List.find_opt(editor => Editor.getBufferId(editor) == bufferId);

      let (isPreview, editor) =
        switch (existingEditor) {
        | Some(ed) =>
          let isPreview = Editor.getPreview(ed) && preview;

          let editor' =
            switch (split) {
            // If we're in the same split, just re-use current editor
            | SplitDirection.Current => ed

            // However, if we're splitting, we need to clone the editor
            | SplitDirection.Horizontal
            | SplitDirection.Vertical(_)
            | SplitDirection.Inactive
            | SplitDirection.NewTab => Editor.copy(ed)
            };

          (isPreview, Editor.setPreview(~preview=isPreview, editor'));
        | None => (
            preview,
            Editor.create(
              ~config=config(~fileType),
              ~buffer=editorBuffer,
              ~preview,
              (),
            ),
          )
        };

      let previousActiveGroup = Feature_Layout.activeGroup(state.layout);

      // Make any splits or changes to the layout based on the current split direction
      let layout =
        switch (split) {
        | SplitDirection.Inactive =>
          let isTerminalFocus =
            switch (FocusManager.current(state)) {
            | Terminal(_) => true
            | _ => false
            };
          // Do we have focus? If not, we can just use the existing split -
          // don't bother going to a different one.
          if (FocusManager.current(state) != Focus.Editor && !isTerminalFocus) {
            state.layout;
          } else {
            let allGroups = Feature_Layout.activeLayoutGroups(state.layout);

            // If there is only one split, make a new one for the editor.
            if (List.length(allGroups) == 1) {
              Feature_Layout.split(
                ~shouldReuse=true,
                ~editor,
                `Horizontal,
                state.layout,
              );
            } else {
              // Otherwise - just switch to an alternate group, temporarily.
              allGroups
              |> List.filter(group =>
                   Feature_Layout.Group.id(group)
                   != Feature_Layout.Group.id(previousActiveGroup)
                 )
              |> Utility.ListEx.nth_opt(0)
              |> Option.map(newGroup => {
                   Feature_Layout.setActiveGroup(
                     Feature_Layout.Group.id(newGroup),
                     state.layout,
                   )
                 })
              |> Option.value(~default=state.layout);
            };
          };
        | SplitDirection.Current => state.layout
        | SplitDirection.Horizontal =>
          Feature_Layout.split(
            ~shouldReuse=false,
            ~editor,
            `Horizontal,
            state.layout,
          )
        | SplitDirection.Vertical({shouldReuse}) =>
          Feature_Layout.split(~shouldReuse, ~editor, `Vertical, state.layout)
        | SplitDirection.NewTab =>
          Feature_Layout.addLayoutTab(~editor, state.layout)
        };

      let editor' =
        position
        |> Option.map(cursorPosition => {
             Feature_Editor.Editor.setMode(
               Vim.Mode.Normal({cursor: cursorPosition}),
               editor,
             )
           })
        |> Option.value(~default=editor);

      // If we are using the 'Inactive' split direction strategy, the active
      // split is temporarily switched to open the editor in a separate split -
      // switch it back after the editor is open.
      let maybeRevertToPrevious = layout => {
        switch (split) {
        | SplitDirection.Inactive =>
          Feature_Layout.setActiveGroup(
            Feature_Layout.Group.id(previousActiveGroup),
            layout,
          )
        | _ => layout
        };
      };

      let layout' =
        layout
        |> Feature_Layout.openEditor(~config=config(~fileType), editor')
        |> maybeRevertToPrevious;

      let cleanLayout =
        isPreview
          ? {
            Feature_Layout.activeGroupEditors(layout')
            |> List.filter(ed =>
                 Editor.getPreview(ed)
                 && Editor.getId(ed) != Editor.getId(editor')
               )
            |> List.fold_left(
                 (acc, ed) => {
                   switch (
                     Feature_Layout.removeEditor(Editor.getId(ed), acc)
                   ) {
                   | Some(lay) => lay
                   | None => acc
                   }
                 },
                 layout',
               );
          }
          : layout';

      let bufferRenderers =
        buffer
        |> Oni_Core.Buffer.getFilePath
        |> OptionEx.flatMap(path => {
             switch (Oni_Core.BufferPath.parse(path)) {
             | ExtensionDetails => Some(BufferRenderer.ExtensionDetails)
             | Terminal({bufferId, _}) =>
               Some(
                 BufferRenderer.Terminal({
                   title: "Terminal",
                   id: bufferId,
                   insertMode: true,
                 }),
               )
             | Version => Some(BufferRenderer.Version)
             | UpdateChangelog => Some(BufferRenderer.UpdateChangelog)
             | Image => Some(BufferRenderer.Image)
             | Welcome when Oni_Core.Buffer.isEmpty(buffer) =>
               Some(BufferRenderer.Welcome)
             | Changelog => Some(BufferRenderer.FullChangelog)
             | FilePath(_) => None
             | DebugInput => Some(BufferRenderer.DebugInput)
             | _ => None
             }
           })
        |> Option.map(renderer => {
             BufferRenderers.setById(
               buffer |> Oni_Core.Buffer.getId,
               renderer,
               state.bufferRenderers,
             )
           })
        |> Option.value(~default=state.bufferRenderers);

      let state' = {...state, bufferRenderers, layout: cleanLayout};

      let state'' =
        if (grabFocus) {
          state' |> FocusManager.push(Editor);
        } else {
          state';
        };

      (state'', Effect.none);
    | BufferSaved({buffer, reason}) =>
      let eff =
        Service_Exthost.Effects.FileSystemEventService.onBufferChanged(
          ~buffer,
          extHostClient,
        );

      let maybeFullPath =
        buffer
        |> Buffer.getFilePath
        |> OptionEx.flatMap(FpExp.absoluteCurrentPlatform);

      let clearSnippetCacheEffect =
        maybeFullPath
        |> Option.map(filePath =>
             Service_Snippets.Effect.clearCachedSnippets(~filePath)
           )
        |> Option.value(~default=Isolinear.Effect.none);

      let modelSavedEff =
        Service_Exthost.Effects.Documents.modelSaved(
          ~uri=Buffer.getUri(buffer), extHostClient, () =>
          Noop
        );

      let input' =
        maybeFullPath
        |> Option.map(fp => Feature_Input.notifyFileSaved(fp, state.input))
        |> Option.value(~default=state.input);

      let config' =
        maybeFullPath
        |> Option.map(fp =>
             Feature_Configuration.notifyFileSaved(fp, state.config)
           )
        |> Option.value(~default=state.config);

      let maybeActiveBufferId =
        state
        |> Selectors.getActiveBuffer
        |> Option.map(Oni_Core.Buffer.getId);

      let (languageSupport', languageEffect') =
        maybeActiveBufferId
        |> Option.map(activeBufferId => {
             let config = Selectors.configResolver(state);
             state.languageSupport
             |> Feature_LanguageSupport.bufferSaved(
                  ~reason,
                  ~isLargeBuffer=
                    Feature_Buffers.isLargeFile(state.buffers, buffer),
                  ~activeBufferId,
                  ~savedBufferId=Oni_Core.Buffer.getId(buffer),
                  ~config,
                  ~buffer,
                );
           })
        |> Option.value(
             ~default=(state.languageSupport, Isolinear.Effect.none),
           );

      (
        {
          ...state,
          input: input',
          config: config',
          languageSupport: languageSupport',
        },
        Isolinear.Effect.batch([
          eff,
          modelSavedEff,
          clearSnippetCacheEffect,
          languageEffect'
          |> Isolinear.Effect.map(msg => Actions.LanguageSupport(msg)),
        ]),
      );

    | BufferIndentationChanged({buffer}) =>
      let bufferId = Buffer.getId(buffer);
      let layout =
        Feature_Layout.map(
          editor =>
            Feature_Editor.(
              if (Editor.getBufferId(editor) == bufferId) {
                // Set buffer recalculates word wrap, which is
                // what we need if the indentation has changed.
                Editor.setBuffer(
                  ~buffer=EditorBuffer.ofBuffer(buffer),
                  editor,
                );
              } else {
                editor;
              }
            ),
          state.layout,
        );

      ({...state, layout}, Isolinear.Effect.none);

    | BufferUpdated({
        update,
        newBuffer,
        oldBuffer,
        triggerKey,
        markerUpdate,
        minimalUpdate,
      }) =>
      let fileType =
        newBuffer |> Buffer.getFileType |> Buffer.FileType.toString;

      let bufferUpdate = update;

      let languageInfo =
        state.languageSupport |> Feature_LanguageSupport.languageInfo;
      let syntaxHighlights =
        Feature_Syntax.handleUpdate(
          ~scope=Internal.getScopeForBuffer(~languageInfo, newBuffer),
          ~grammars=grammarRepository,
          ~config=
            Feature_Configuration.resolver(
              ~fileType,
              state.config,
              state.vim,
            ),
          ~theme=state.colorTheme |> Feature_Theme.tokenColors,
          ~bufferUpdate=update,
          ~markerUpdate,
          state.syntaxHighlights,
        );

      let diagnostics =
        Feature_Diagnostics.moveMarkers(
          ~newBuffer,
          ~markerUpdate,
          state.diagnostics,
        );

      let languageSupport =
        Feature_LanguageSupport.moveMarkers(
          ~newBuffer,
          ~markerUpdate,
          state.languageSupport,
        );

      let bufferRenderers =
        BufferRenderers.handleBufferUpdate(update, state.bufferRenderers);

      let vim = Feature_Vim.moveMarkers(~newBuffer, ~markerUpdate, state.vim);

      let state' = {
        ...state,
        bufferRenderers,
        syntaxHighlights,
        diagnostics,
        languageSupport,
        vim,
      };

      let syntaxEffect =
        Feature_Syntax.Effect.bufferUpdate(
          ~bufferUpdate=update,
          state.syntaxHighlights,
        )
        |> Isolinear.Effect.map(() => Actions.Noop);

      let exthostEffect =
        Service_Exthost.Effects.Documents.modelChanged(
          ~previousBuffer=oldBuffer,
          ~buffer=newBuffer,
          ~update,
          ~minimalUpdate,
          extHostClient,
          () =>
          Actions.ExtensionBufferUpdateQueued({triggerKey: triggerKey})
        );
      open Feature_Editor; // update editor

      let buffer = EditorBuffer.ofBuffer(newBuffer);
      let bufferId = Buffer.getId(newBuffer);
      (
        {
          ...state',
          layout:
            Feature_Layout.map(
              editor =>
                if (Editor.getBufferId(editor) == bufferId) {
                  Editor.updateBuffer(
                    ~update=bufferUpdate,
                    ~markerUpdate,
                    ~buffer,
                    editor,
                  );
                } else {
                  editor;
                },
              state'.layout,
            ),
        },
        Isolinear.Effect.batch([exthostEffect, syntaxEffect]),
      );
    };

  | EditorSizeChanged({id, pixelWidth, pixelHeight}) => (
      {
        ...state,
        layout:
          Feature_Layout.map(
            editor =>
              Feature_Editor.Editor.getId(editor) == id
                ? Feature_Editor.Editor.setSize(
                    ~pixelWidth,
                    ~pixelHeight,
                    editor,
                  )
                : editor,
            state.layout,
          ),
      },
      Effect.none,
    )

  | Configuration(msg) =>
    let (config, outmsg) = Feature_Configuration.update(state.config, msg);
    let state = {...state, config};
    switch (outmsg) {
    | ConfigurationChanged({changed}) =>
      let vsyncEffect =
        if (Config.Settings.get(Config.key("vsync"), changed) != None) {
          Isolinear.Effect.create(~name="Features.setVsync", () => {
            let resolver = Selectors.configResolver(state);
            setVsync(
              Feature_Configuration.GlobalConfiguration.vsync.get(resolver),
            );
          });
        } else {
          Isolinear.Effect.none;
        };

      let (state', configurationEffect) =
        state |> Internal.updateConfiguration;
      (state', Isolinear.Effect.batch([configurationEffect, vsyncEffect]));

    | OpenFile(fp) => (state, Internal.openFileEffect(FpExp.toString(fp)))

    | Nothing => (state, Effect.none)
    };

  | Commands(msg) =>
    let commands = Feature_Commands.update(state.commands, msg);
    let state = {...state, commands};
    (state, Effect.none);

  | Syntax(msg) =>
    let (syntaxHighlights, out) =
      Feature_Syntax.update(state.syntaxHighlights, msg);
    let state = {...state, syntaxHighlights};

    let effect =
      switch (out) {
      | Nothing => Effect.none
      | ServerError(msg) =>
        Internal.notificationEffect(
          ~kind=Error,
          "Syntax Server error: " ++ msg,
        )
      };
    (state, effect);

  | Layout(msg) =>
    open Feature_Layout;

    let sideBarLocation = Feature_SideBar.location(state.sideBar);

    let focus =
      switch (FocusManager.current(state)) {
      | Pane => Some(Bottom)

      | Editor
      | Terminal(_) => Some(Center)

      | Extensions
      | FileExplorer
      | SCM
      | Search when sideBarLocation == Feature_SideBar.Left => Some(Left)

      | Extensions
      | FileExplorer
      | SCM
      | Search when sideBarLocation == Feature_SideBar.Right => Some(Right)

      | _ => None
      };

    let (model, outmsg) = update(~focus, state.layout, msg);
    let state = {...state, layout: model};

    switch (outmsg) {
    | Focus(Center) => (FocusManager.push(Editor, state), Effect.none)

    | Focus(Left) when sideBarLocation == Feature_SideBar.Left => (
        switch (state.sideBar |> Feature_SideBar.selected) {
        | FileExplorer => FocusManager.push(FileExplorer, state)
        | SCM => FocusManager.push(SCM, state)
        | Extensions => FocusManager.push(Extensions, state)
        | Search => FocusManager.push(Search, state)
        },
        Effect.none,
      )

    | Focus(Right) when sideBarLocation == Feature_SideBar.Right => (
        switch (state.sideBar |> Feature_SideBar.selected) {
        | FileExplorer => FocusManager.push(FileExplorer, state)
        | SCM => FocusManager.push(SCM, state)
        | Extensions => FocusManager.push(Extensions, state)
        | Search => FocusManager.push(Search, state)
        },
        Effect.none,
      )

    | Focus(Right)
    | Focus(Left) => (state, Effect.none)

    | Focus(Bottom) => (state |> FocusManager.push(Pane), Effect.none)

    | SplitAdded => (
        {...state, zen: Feature_Zen.exitZenMode(state.zen)},
        Effect.none,
      )

    | RemoveLastWasBlocked =>
      // #417: If there are any unsaved files... show a warning and give the user a chance to save.
      if (Feature_Buffers.anyModified(state.buffers)) {
        (
          {...state, modal: Some(Feature_Modals.unsavedBuffersWarning)},
          Isolinear.Effect.none,
        );
      } else {
        (state, Internal.quitEffect);
      }

    | Nothing => (state, Effect.none)
    };

  | Terminal(msg) =>
    let (model, eff) =
      Feature_Terminal.update(
        ~clientServer=state.clientServer,
        ~config=Selectors.configResolver(state),
        state.terminals,
        msg,
      );

    let state = {...state, terminals: model};

    let (state, effect) =
      switch ((eff: Feature_Terminal.outmsg)) {
      | Nothing => (state, Effect.none)
      | Effect(eff) => (
          state,
          eff |> Effect.map(msg => Actions.Terminal(msg)),
        )

      | NotifyError(msg) => (
          state,
          Internal.notificationEffect(~kind=Error, msg),
        )

      | ClosePane({paneId}) => (
          state,
          Feature_Pane.Msg.close(~paneId)
          |> EffectEx.value(~name="Feature_Terminal.closePane")
          |> Effect.map(msg => Pane(msg)),
        )
      | TogglePane({paneId}) => (
          state,
          Feature_Pane.Msg.toggle(~paneId)
          |> EffectEx.value(~name="Feature_Terminal.togglePane")
          |> Effect.map(msg => Pane(msg)),
        )

      | SwitchToNormalMode =>
        let maybeBufferId =
          state
          |> Selectors.getActiveBuffer
          |> Option.map(Oni_Core.Buffer.getId);

        let maybeTerminalId =
          maybeBufferId
          |> Option.map(id =>
               BufferRenderers.getById(id, state.bufferRenderers)
             )
          |> OptionEx.flatMap(
               fun
               | BufferRenderer.Terminal({id, _}) => Some(id)
               | _ => None,
             );

        let editorId =
          Feature_Layout.activeEditor(state.layout)
          |> Feature_Editor.Editor.getId;

        let (state, effect) =
          OptionEx.map2(
            (bufferId, terminalId) => {
              let colorTheme = Feature_Theme.colors(state.colorTheme);
              let (lines, highlights) =
                Feature_Terminal.getLinesAndHighlights(
                  ~colorTheme,
                  ~terminalId,
                );
              let syntaxHighlights =
                List.fold_left(
                  (acc, curr) => {
                    let (line, tokens) = curr;
                    Feature_Syntax.setTokensForLine(
                      ~bufferId,
                      ~line,
                      ~tokens,
                      acc,
                    );
                  },
                  state.syntaxHighlights,
                  highlights,
                );

              let syntaxHighlights =
                syntaxHighlights |> Feature_Syntax.ignore(~bufferId);

              let terminalFont = Feature_Terminal.font(state.terminals);
              let layout =
                Feature_Layout.map(
                  editor =>
                    if (Feature_Editor.Editor.getBufferId(editor) == bufferId) {
                      state.buffers
                      |> Feature_Buffers.get(bufferId)
                      |> Option.map(buffer => {
                           let updatedBuffer =
                             buffer
                             |> Oni_Core.Buffer.setFont(terminalFont)
                             |> Feature_Editor.EditorBuffer.ofBuffer;
                           Feature_Editor.Editor.setBuffer(
                             ~buffer=updatedBuffer,
                             editor,
                           );
                         })
                      |> Option.value(~default=editor);
                    } else {
                      editor;
                    },
                  state.layout,
                );

              (
                {...state, layout, syntaxHighlights},
                Feature_Vim.Effects.setTerminalLines(
                  ~bufferId,
                  ~editorId,
                  lines,
                )
                |> Isolinear.Effect.map(msg => Actions.Vim(msg)),
              );
            },
            maybeBufferId,
            maybeTerminalId,
          )
          |> Option.value(~default=(state, Isolinear.Effect.none));

        (state, effect);

      | TerminalCreated({name, splitDirection}) =>
        let windowTreeDirection = splitDirection;

        let eff =
          Isolinear.Effect.createWithDispatch(
            ~name="feature.terminal.openBuffer", dispatch => {
            dispatch(Actions.OpenFileByPath(name, windowTreeDirection, None))
          });
        (state, eff);

      | TerminalExit({terminalId, shouldClose, _}) when shouldClose == true =>
        switch (Selectors.getBufferForTerminal(~terminalId, state)) {
        | Some(buffer) =>
          switch (
            Feature_Layout.closeBuffer(~force=true, buffer, state.layout)
          ) {
          | Some(layout) => ({...state, layout}, Effect.none)
          | None => (state, Internal.quitEffect)
          }
        | None => (state, Effect.none)
        }

      | TerminalExit(_) => (state, Effect.none)
      };

    (state, effect);

  | OpenBufferById({bufferId, direction}) =>
    let languageInfo =
      state.languageSupport |> Feature_LanguageSupport.languageInfo;
    let effect =
      Feature_Buffers.Effects.openBufferInEditor(
        ~languageInfo,
        ~font=state.editorFont,
        ~bufferId,
        ~split=direction,
        state.buffers,
      )
      |> Isolinear.Effect.map(msg => Actions.Buffers(msg));
    (state, effect);

  | NewBuffer({direction}) =>
    let languageInfo =
      state.languageSupport |> Feature_LanguageSupport.languageInfo;
    let effect =
      Feature_Buffers.Effects.openNewBuffer(
        ~split=direction,
        ~languageInfo,
        ~font=state.editorFont,
        state.buffers,
      )
      |> Isolinear.Effect.map(msg => Actions.Buffers(msg));
    (state, effect);

  | OpenFileByPath(filePath, split, position) =>
    let languageInfo =
      state.languageSupport |> Feature_LanguageSupport.languageInfo;
    let effect =
      Feature_Buffers.Effects.openFileInEditor(
        ~languageInfo,
        ~font=state.editorFont,
        ~split,
        ~position,
        ~grabFocus=true,
        ~filePath,
        state.buffers,
      )
      |> Isolinear.Effect.map(msg => Actions.Buffers(msg));
    (state, effect);
  | PreviewFileByPath(filePath, split, position) =>
    let languageInfo =
      state.languageSupport |> Feature_LanguageSupport.languageInfo;
    let effect =
      Feature_Buffers.Effects.openFileInEditor(
        ~languageInfo,
        ~font=state.editorFont,
        ~split,
        ~position,
        ~grabFocus=true,
        ~filePath,
        ~preview=true,
        state.buffers,
      )
      |> Isolinear.Effect.map(msg => Actions.Buffers(msg));
    (state, effect);

  | Theme(msg) =>
    let (model', outmsg) = Feature_Theme.update(state.colorTheme, msg);

    let state = {...state, colorTheme: model'};
    switch (outmsg) {
    | ConfigurationTransform(transformer) => (
        {
          ...state,
          config:
            Feature_Configuration.queueTransform(~transformer, state.config),
        },
        Isolinear.Effect.none,
      )
    | NotifyError(msg) => (
        state,
        Internal.notificationEffect(~kind=Error, msg),
      )

    | OpenThemePicker(_) =>
      let themes =
        state.extensions
        |> Feature_Extensions.pick((manifest: Exthost.Extension.Manifest.t) => {
             Exthost.Extension.Contributions.(manifest.contributes.themes)
           })
        |> List.flatten;

      (state |> Internal.showThemePicker(themes), Isolinear.Effect.none);
    | Nothing => (state, Isolinear.Effect.none)
    | ThemeChanged(_colorTheme) =>
      let config = Selectors.configResolver(state);
      let theme = Feature_Theme.colors(state.colorTheme);
      (
        {
          ...state,
          notifications:
            Feature_Notification.changeTheme(
              ~config,
              ~theme,
              state.notifications,
            ),
        },
        Isolinear.Effect.none,
      );
    };

  | Notification(msg) =>
    let config = Selectors.configResolver(state);
    let theme = Feature_Theme.colors(state.colorTheme);
    let model' =
      Feature_Notification.update(~theme, ~config, state.notifications, msg);
    // let pane' = Feature_Pane.setNotifications(model', state.pane);
    let pane' = state.pane;
    ({...state, notifications: model', pane: pane'}, Effect.none);

  | Modals(msg) =>
    switch (state.modal) {
    | Some(model) =>
      let (model, outmsg) = Feature_Modals.update(model, msg);

      switch (outmsg) {
      | ChoiceConfirmed(eff) => (
          {...state, modal: None},
          eff |> Effect.map(msg => Actions.Modals(msg)),
        )

      | Effect(eff) => (
          {...state, modal: Some(model)},
          eff |> Effect.map(msg => Actions.Modals(msg)),
        )
      };

    | None => (state, Effect.none)
    }

  | FilesDropped({paths}) =>
    let eff =
      Service_OS.Effect.statMultiple(paths, (~exists, ~isDirectory, path) =>
        if (!exists || !isDirectory) {
          OpenFileByPath(path, SplitDirection.Current, None);
        } else if (isDirectory) {
          switch (Luv.Path.chdir(path)) {
          | Ok () =>
            Actions.Workspace(
              Feature_Workspace.Msg.workingDirectoryChanged(path),
            )
          | Error(_) => Noop
          };
        } else {
          OpenFileByPath(path, SplitDirection.Current, None);
        }
      );
    (state, eff);

  | Editor({scope, msg}) =>
    let (layout, effect) =
      Internal.updateEditors(
        ~client=extHostClient,
        ~scope,
        ~msg,
        state.layout,
      );
    let state = {...state, layout};
    (state, effect);

  | Changelog(msg) =>
    let (model, eff) = Feature_Changelog.update(state.changelog, msg);
    ({...state, changelog: model}, eff);

  // TODO: This should live in the editor feature project
  | EditorFont(Service_Font.FontLoaded(font)) =>
    let buffers' =
      state.buffers |> Feature_Buffers.map(Oni_Core.Buffer.setFont(font));
    (
      {
        ...state,
        editorFont: font,
        buffers: buffers',
        layout:
          Feature_Layout.map(
            editor => {
              let bufferId = Feature_Editor.Editor.getBufferId(editor);
              buffers'
              |> Feature_Buffers.get(bufferId)
              |> Option.map(buffer => {
                   let updatedBuffer =
                     buffer |> Feature_Editor.EditorBuffer.ofBuffer;
                   Feature_Editor.Editor.setBuffer(
                     ~buffer=updatedBuffer,
                     editor,
                   );
                 })
              |> Option.value(~default=editor);
            },
            state.layout,
          ),
      },
      Effect.none,
    );
  | EditorFont(Service_Font.FontLoadError(message)) => (
      state,
      Internal.notificationEffect(~kind=Error, message),
    )

  | Output(msg) =>
    let (output', outmsg) = Feature_Output.update(msg, state.output);

    let state' = {...state, output: output'};
    switch (outmsg) {
    | Nothing => (state', Isolinear.Effect.none)

    | ClosePane => (
        {...state', pane: Feature_Pane.close(state'.pane)},
        Isolinear.Effect.none,
      )
    };

  | Quickmenu(msg) =>
    let (quickmenu', outmsg) =
      Feature_Quickmenu.update(msg, state.newQuickmenu);
    let eff =
      switch (outmsg) {
      | Nothing => Isolinear.Effect.none

      | Action(action) =>
        EffectEx.value(~name="Feature_Quickmenu.action", action)
      };

    ({...state, newQuickmenu: quickmenu'}, eff);

  | QuickOpen(msg) =>
    let (quickOpen', outmsg) =
      Feature_QuickOpen.update(~client=extHostClient, msg, state.quickOpen);

    let (state', eff) =
      switch (outmsg) {
      | Nothing => (state, Isolinear.Effect.none)
      | Effect(eff) => (
          state,
          eff |> Isolinear.Effect.map(msg => Actions.QuickOpen(msg)),
        )
      | ShowMenu(menu) =>
        let menu' =
          menu |> Feature_Quickmenu.Schema.map(msg => Actions.QuickOpen(msg));
        let quickmenu' =
          Feature_Quickmenu.show(~menu=menu', state.newQuickmenu);
        ({...state, newQuickmenu: quickmenu'}, Isolinear.Effect.none);
      };

    ({...state', quickOpen: quickOpen'}, eff);

  | Snippets(msg) =>
    let maybeBuffer = Selectors.getActiveBuffer(state);
    let editor = Feature_Layout.activeEditor(state.layout);
    let editorId = editor |> Feature_Editor.Editor.getId;

    let config = Selectors.configResolver(state);
    let cursorPosition = editor |> Feature_Editor.Editor.getPrimaryCursorByte;

    let resolverFactory = () => {
      Oni_Model.SnippetVariables.current(state);
    };

    let wasSnippetActive = Feature_Snippets.isActive(state.snippets);

    let selections = Feature_Editor.Editor.selections(editor);
    let languageInfo =
      state.languageSupport |> Feature_LanguageSupport.languageInfo;

    let (snippets', outmsg) =
      Feature_Snippets.update(
        ~languageInfo,
        ~resolverFactory,
        ~selections,
        ~maybeBuffer,
        ~editorId,
        ~cursorPosition,
        ~extensions=state.extensions,
        msg,
        state.snippets,
      );

    let isSnippetActive = Feature_Snippets.isActive(snippets');

    let updateLanguageSupport = (languageSupport, oldLayout, newLayout) => {
      let originalEditor = Feature_Layout.activeEditor(oldLayout);
      let originalCursor =
        originalEditor |> Feature_Editor.Editor.getPrimaryCursor;

      let originalMode = originalEditor |> Feature_Editor.Editor.mode;
      let wasInInsert = Vim.Mode.isInsertOrSelect(originalMode);

      let newEditor = Feature_Layout.activeEditor(newLayout);
      let newCursor = newEditor |> Feature_Editor.Editor.getPrimaryCursor;
      let newMode = newEditor |> Feature_Editor.Editor.mode;
      let isInInsert = Vim.Mode.isInsertOrSelect(newMode);
      let languageInfo =
        state.languageSupport |> Feature_LanguageSupport.languageInfo;

      let languageSupport' =
        if (originalCursor != newCursor) {
          maybeBuffer
          |> Option.map(buffer => {
               let languageConfiguration =
                 buffer
                 |> Oni_Core.Buffer.getFileType
                 |> Oni_Core.Buffer.FileType.toString
                 |> Exthost.LanguageInfo.getLanguageConfiguration(
                      languageInfo,
                    )
                 |> Option.value(~default=LanguageConfiguration.default);
               Feature_LanguageSupport.cursorMoved(
                 ~editorId=Feature_Editor.Editor.getId(newEditor),
                 ~languageConfiguration,
                 ~buffer,
                 ~previous=originalCursor,
                 ~current=newCursor,
                 state.languageSupport,
               );
             })
          |> Option.value(~default=state.languageSupport);
        } else {
          languageSupport;
        };

      let languageSupport'' =
        if (wasInInsert != isInInsert) {
          if (isInInsert) {
            Feature_LanguageSupport.startInsertMode(
              ~config,
              ~maybeBuffer,
              languageSupport',
            );
          } else {
            Feature_LanguageSupport.stopInsertMode(languageSupport');
          };
        } else {
          languageSupport';
        };

      if (wasSnippetActive != isSnippetActive) {
        if (isSnippetActive) {
          Feature_LanguageSupport.startSnippet(languageSupport'');
        } else {
          Feature_LanguageSupport.stopSnippet(languageSupport'');
        };
      } else {
        languageSupport'';
      };
    };

    let (quickmenu', layout', eff) =
      switch (outmsg) {
      | Nothing => (state.newQuickmenu, state.layout, Isolinear.Effect.none)
      | ErrorMessage(msg) => (
          state.newQuickmenu,
          state.layout,
          Internal.notificationEffect(~kind=Error, msg),
        )
      | SetCursors(cursors) =>
        let layout' =
          state.layout
          |> Feature_Layout.map(editor =>
               Feature_Editor.(
                 if (Editor.getId(editor) == editorId) {
                   Editor.setCursors(cursors, editor);
                 } else {
                   editor;
                 }
               )
             );
        (state.newQuickmenu, layout', Isolinear.Effect.none);
      | SetSelections(ranges) =>
        let layout' =
          state.layout
          |> Feature_Layout.map(editor =>
               Feature_Editor.(
                 if (Editor.getId(editor) == editorId) {
                   Editor.setSelections(ranges, editor);
                 } else {
                   editor;
                 }
               )
             );
        (state.newQuickmenu, layout', Isolinear.Effect.none);

      | ShowMenu(snippetMenu) =>
        let menu =
          snippetMenu
          |> Feature_Quickmenu.Schema.map(msg => Actions.Snippets(msg));
        let quickmenu' = Feature_Quickmenu.show(~menu, state.newQuickmenu);
        (quickmenu', state.layout, Isolinear.Effect.none);

      | OpenFile(filePath) => (
          state.newQuickmenu,
          state.layout,
          Internal.openFileEffect(FpExp.toString(filePath)),
        )

      | Effect(eff) => (
          state.newQuickmenu,
          state.layout,
          eff |> Isolinear.Effect.map(msg => Actions.Snippets(msg)),
        )
      };

    let languageSupport' =
      updateLanguageSupport(state.languageSupport, state.layout, layout');
    (
      {
        ...state,
        layout: layout',
        languageSupport: languageSupport',
        newQuickmenu: quickmenu',
        snippets: snippets',
      },
      eff,
    );

  | TitleBar(titleBarMsg) =>
    let (titleBar', outmsg) =
      Feature_TitleBar.update(
        ~maximize,
        ~minimize,
        ~close,
        ~restore,
        titleBarMsg,
        state.titleBar,
      );
    let eff =
      switch (outmsg) {
      | Feature_TitleBar.Effect(effect) => effect
      | Feature_TitleBar.Nothing => Isolinear.Effect.none
      };

    (
      {...state, titleBar: titleBar'},
      eff |> Isolinear.Effect.map(msg => TitleBar(msg)),
    );

  | ExtensionBufferUpdateQueued({triggerKey}) =>
    let maybeBuffer = Selectors.getActiveBuffer(state);
    let editor = Feature_Layout.activeEditor(state.layout);
    let activeCursor = editor |> Feature_Editor.Editor.getPrimaryCursor;
    let activeCursorByte =
      editor |> Feature_Editor.Editor.getPrimaryCursorByte;
    let languageInfo =
      state.languageSupport |> Feature_LanguageSupport.languageInfo;

    let languageSupport' =
      maybeBuffer
      |> Option.map(buffer => {
           let syntaxScope =
             Feature_Syntax.getSyntaxScope(
               ~bufferId=Buffer.getId(buffer),
               ~bytePosition=activeCursorByte,
               state.syntaxHighlights,
             );
           let config = Selectors.configResolver(state);

           let languageConfiguration =
             buffer
             |> Oni_Core.Buffer.getFileType
             |> Oni_Core.Buffer.FileType.toString
             |> Exthost.LanguageInfo.getLanguageConfiguration(languageInfo)
             |> Option.value(~default=LanguageConfiguration.default);

           let languageSupport =
             Feature_LanguageSupport.bufferUpdated(
               ~languageConfiguration,
               ~extensions=state.extensions,
               ~buffer,
               ~config,
               ~activeCursor,
               ~syntaxScope,
               ~triggerKey,
               state.languageSupport,
             );

           languageSupport;
         })
      |> Option.value(~default=state.languageSupport);

    ({...state, languageSupport: languageSupport'}, Isolinear.Effect.none);

  | Yank({range}) =>
    open EditorCoreTypes;
    open Feature_Editor;

    let activeEditor = state.layout |> Feature_Layout.activeEditor;
    let activeEditorId = activeEditor |> Editor.getId;
    let maybeBuffer = Selectors.getActiveBuffer(state);

    switch (maybeBuffer) {
    | None => (state, Isolinear.Effect.none)
    | Some(buffer) =>
      let byteRanges = Selection.getRanges(range, buffer);

      let cursorLine =
        activeEditor
        |> Editor.getPrimaryCursor
        |> (
          ({line, _}: CharacterPosition.t) =>
            line |> EditorCoreTypes.LineNumber.toZeroBased
        );

      let maxDelta = 400;

      let pixelRanges =
        byteRanges
        |> List.filter(({start, _}: ByteRange.t) => {
             let lineNumber =
               start.line |> EditorCoreTypes.LineNumber.toZeroBased;
             abs(lineNumber - cursorLine) < maxDelta;
           })
        |> List.map(({start, stop}: ByteRange.t) => {
             let (pixelStart, _) =
               Editor.bufferBytePositionToPixel(
                 ~position=start,
                 activeEditor,
               );

             let (pixelStop, _) =
               Editor.bufferBytePositionToPixel(~position=stop, activeEditor);
             let lineHeightInPixels =
               activeEditor |> Editor.lineHeightInPixels;
             let range =
               PixelRange.create(
                 ~start=pixelStart,
                 ~stop={
                   x: pixelStop.x,
                   y: pixelStart.y +. lineHeightInPixels,
                 },
               );
             range;
           });
      let layout' =
        state.layout
        |> Feature_Layout.map(editor =>
             if (Editor.getId(editor) == activeEditorId) {
               Editor.startYankHighlight(pixelRanges, editor);
             } else {
               editor;
             }
           );
      ({...state, layout: layout'}, Isolinear.Effect.none);
    };

  | Vim(msg) =>
    let previousSubMode = state.vim |> Feature_Vim.subMode;
    let vimContext = VimContext.current(state);
    let (vim, outmsg) = Feature_Vim.update(~vimContext, msg, state.vim);
    let newSubMode = state.vim |> Feature_Vim.subMode;

    // If we've switched to, or from, insert literal,
    // we may need to enable/disable key bindings.
    let input' =
      if (previousSubMode != newSubMode) {
        if (newSubMode == Vim.SubMode.InsertLiteral) {
          Feature_Input.disable(state.input);
        } else {
          Feature_Input.enable(state.input);
        };
      } else {
        state.input;
      };

    let state = {...state, input: input', vim};

    switch (outmsg) {
    | Nothing => (state, Isolinear.Effect.none)
    | Effect(e) => (
        state,
        e |> Isolinear.Effect.map(msg => Actions.Vim(msg)),
      )
    | SettingsChanged({name, value}) =>
      let maybeActiveBuffer = Selectors.getActiveBuffer(state);
      let bufferEffects =
        maybeActiveBuffer
        |> Option.map(activeBuffer => {
             Feature_Buffers.vimSettingChanged(
               ~activeBufferId=Buffer.getId(activeBuffer),
               ~name,
               ~value,
               state.buffers,
             )
             |> Isolinear.Effect.map(msg => Buffers(msg))
           })
        |> Option.value(~default=Isolinear.Effect.none);

      let (state, eff) = state |> Internal.updateConfiguration;

      (state, Isolinear.Effect.batch([eff, bufferEffects]));

    | ModeDidChange({allowAnimation, mode, effects}) =>
      Internal.updateMode(
        ~client=extHostClient,
        ~allowAnimation,
        state,
        mode,
        effects,
      )
    | Output({cmd, output}) =>
      let output' =
        state.output |> Feature_Output.setProcessOutput(~cmd, ~output);
      let pane' =
        state.pane |> Feature_Pane.show(~paneId="workbench.panel.output");
      (
        {...state, pane: pane', output: output'} |> FocusManager.push(Pane),
        Isolinear.Effect.none,
      );
    };

  | Workspace(msg) =>
    let (workspace, outmsg) = Feature_Workspace.update(msg, state.workspace);

    let state = {...state, workspace};
    switch (outmsg) {
    | Nothing => (state, Isolinear.Effect.none)

    | Effect(eff) => (
        state,
        eff |> Isolinear.Effect.map(msg => Workspace(msg)),
      )

    | WorkspaceChanged({path: maybeWorkspaceFolder, shouldFocusExplorer}) =>
      let maybeExplorerFolder =
        maybeWorkspaceFolder
        |> OptionEx.flatMap(FpExp.absoluteCurrentPlatform);
      let fileExplorer =
        Feature_Explorer.setRoot(
          ~rootPath=maybeExplorerFolder,
          state.fileExplorer,
        );

      // Pop open and focus sidebar
      let sideBar =
        if (shouldFocusExplorer
            && (
              !Feature_SideBar.isOpen(state.sideBar)
              || Feature_SideBar.selected(state.sideBar)
              !== Feature_SideBar.FileExplorer
            )) {
          Feature_SideBar.toggle(FileExplorer, state.sideBar);
        } else {
          state.sideBar;
        };

      let extWorkspace =
        maybeWorkspaceFolder |> Option.map(Exthost.WorkspaceData.fromPath);
      let eff =
        Service_Exthost.Effects.Workspace.change(
          ~workspace=extWorkspace,
          extHostClient,
        );

      let state' =
        if (shouldFocusExplorer) {
          {...state, fileExplorer, sideBar}
          |> FocusManager.push(Focus.FileExplorer);
        } else {
          {...state, fileExplorer, sideBar};
        };
      (state', eff);
    };

  | Zen(msg) =>
    let zen' = Feature_Zen.update(msg, state.zen);
    ({...state, zen: zen'}, Isolinear.Effect.none);

  | Zoom(msg) =>
    let (zoom', outmsg) = Feature_Zoom.update(msg, state.zoom);
    let state' = {...state, zoom: zoom'};
    let (state'', eff) =
      switch (outmsg) {
      | Feature_Zoom.Nothing => (state', Isolinear.Effect.none)
      | Feature_Zoom.UpdateConfiguration(transformer) => (
          {
            ...state',
            config:
              Feature_Configuration.queueTransform(
                ~transformer,
                state'.config,
              ),
          },
          Isolinear.Effect.none,
        )
      | Feature_Zoom.Effect(eff) => (
          state',
          eff |> Isolinear.Effect.map(msg => Actions.Zoom(msg)),
        )
      };
    (state'', eff);

  | AutoUpdate(msg) =>
    let getLicenseKey = () =>
      Feature_Registration.getLicenseKey(state.registration);
    let (state', outmsg) =
      Feature_AutoUpdate.update(~getLicenseKey, state.autoUpdate, msg);

    let eff =
      switch (outmsg) {
      | Nothing => Isolinear.Effect.none
      | Effect(eff) =>
        eff |> Isolinear.Effect.map(msg => Actions.AutoUpdate(msg))
      | ErrorMessage(msg) => Internal.notificationEffect(~kind=Error, msg)
      };

    ({...state, autoUpdate: state'}, eff);

  | _ => (state, Effect.none)
  };

// SUBSCRIPTIONS

module QuickmenuSubscriptionRunner =
  Subscription.Runner({
    type action = Actions.t;
    let id = "quickmenu-subscription";
  });

let updateSubscriptions = (setup: Setup.t) => {
  let ripgrep = Ripgrep.make(~executablePath=setup.rgPath);

  let quickmenuSubscriptions = QuickmenuStoreConnector.subscriptions(ripgrep);

  (state: State.t, dispatch) => {
    quickmenuSubscriptions(dispatch, state)
    |> QuickmenuSubscriptionRunner.run(~dispatch);
  };
};
