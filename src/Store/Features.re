open Isolinear;
open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Actions;

module Internal = {
  let notificationEffect = (~kind, message) => {
    Feature_Notification.Effects.create(~kind, message)
    |> Isolinear.Effect.map(msg => Actions.Notification(msg));
  };

  let openFileEffect = (~position=None, filePath) => {
    Isolinear.Effect.createWithDispatch(
      ~name="features.openFileByPath", dispatch =>
      dispatch(OpenFileByPath(filePath, None, position))
    );
  };

  let setThemesEffect =
      (~themes: list(Exthost.Extension.Contributions.Theme.t)) => {
    switch (themes) {
    | [] => Isolinear.Effect.none
    | [theme] =>
      Isolinear.Effect.createWithDispatch(
        ~name="feature.extensions.selectTheme", dispatch => {
        dispatch(ThemeLoadByName(theme.label))
      })
    | themes =>
      Isolinear.Effect.createWithDispatch(
        ~name="feature.extensions.showThemeAfterInstall", dispatch => {
        dispatch(QuickmenuShow(ThemesPicker(themes)))
      })
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
      dispatch(Actions.Quit(true))
    );

  let updateEditor = (~editorId, ~msg, layout) => {
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
        | MouseHovered({characterPosition, _}) =>
          Effect.createWithDispatch(~name="editor.mousehovered", dispatch => {
            dispatch(
              LanguageSupport(
                Feature_LanguageSupport.Msg.Hover.mouseHovered(
                  characterPosition,
                ),
              ),
            )
          })
        | MouseMoved({characterPosition, _}) =>
          Effect.createWithDispatch(~name="editor.mousemoved", dispatch => {
            dispatch(
              LanguageSupport(
                Feature_LanguageSupport.Msg.Hover.mouseMoved(
                  characterPosition,
                ),
              ),
            )
          })
        };

      (layout, effect);
    | None => (layout, Effect.none)
    };
  };

  let updateEditors =
      (
        ~scope: EditorScope.t,
        ~msg: Feature_Editor.msg,
        layout: Feature_Layout.model,
      ) => {
    switch (scope) {
    | All =>
      let (layout', effects) =
        Feature_Layout.fold(
          (prev, editor) => {
            let (layout, effects) = prev;
            let editorId = Feature_Editor.Editor.getId(editor);
            let (layout', effect') = updateEditor(~editorId, ~msg, layout);
            (layout', [effect', ...effects]);
          },
          (layout, []),
          layout,
        );
      (layout', Isolinear.Effect.batch(effects));
    | Editor(editorId) => updateEditor(~editorId, ~msg, layout)
    };
  };
};

// UPDATE

let update =
    (
      ~grammarRepository: Oni_Syntax.GrammarRepository.t,
      ~extHostClient: Exthost.Client.t,
      ~getUserSettings,
      ~setup,
      ~maximize,
      ~minimize,
      ~close,
      ~restore,
      state: State.t,
      action: Actions.t,
    ) =>
  switch (action) {
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
    let diagnostics = Feature_Diagnostics.update(msg, state.diagnostics);
    (
      {
        ...state,
        diagnostics,
        pane: Feature_Pane.setDiagnostics(diagnostics, state.pane),
      },
      Isolinear.Effect.none,
    );

  | Exthost(msg) =>
    let (model, outMsg) = Feature_Exthost.update(msg, state.exthost);

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
      Feature_Extensions.update(~extHostClient, msg, state.extensions);
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

        | SelectTheme({themes}) =>
          let eff = Internal.setThemesEffect(~themes);
          (state, eff);

        | UnhandledWindowMovement(movement) => (
            state,
            Internal.unhandledWindowMotionEffect(movement),
          )

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
          let showThemePickerEffect = Internal.setThemesEffect(~themes);
          (
            state,
            Isolinear.Effect.batch([
              notificationEffect,
              showThemePickerEffect,
            ]),
          );
        }
      );
    (state', effect);

  | FileExplorer(msg) =>
    let (model, outmsg) =
      Feature_Explorer.update(
        ~configuration=state.configuration,
        ~languageInfo=state.languageInfo,
        ~iconTheme=state.iconTheme,
        msg,
        state.fileExplorer,
      );

    let state = {...state, fileExplorer: model};

    Feature_Explorer.(
      switch (outmsg) {
      | Nothing => (state, Isolinear.Effect.none)
      | Effect(effect) => (
          state,
          effect |> Isolinear.Effect.map(msg => FileExplorer(msg)),
        )
      | OpenFile(filePath) => (state, Internal.openFileEffect(filePath))
      | GrabFocus => (
          FocusManager.push(Focus.FileExplorer, state),
          Isolinear.Effect.none,
        )
      }
    );

  | Input(msg) =>
    let (model, outmsg) = Feature_Input.update(msg, state.input);

    let eff =
      switch (outmsg) {
      | Nothing => Isolinear.Effect.none
      | DebugInputShown => Internal.openFileEffect("oni://DebugInput")
      };

    ({...state, input: model}, eff);

  | LanguageSupport(msg) =>
    let maybeBuffer = Oni_Model.Selectors.getActiveBuffer(state);
    let editor = state.layout |> Feature_Layout.activeEditor;
    let cursorLocation = editor |> Feature_Editor.Editor.getPrimaryCursor;

    let selection = editor |> Feature_Editor.Editor.selectionOrCursorRange;

    let editorId = editor |> Feature_Editor.Editor.getId;

    let languageConfiguration =
      maybeBuffer
      |> Option.map(Oni_Core.Buffer.getFileType)
      |> Option.map(Oni_Core.Buffer.FileType.toString)
      |> OptionEx.flatMap(
           Exthost.LanguageInfo.getLanguageConfiguration(state.languageInfo),
         )
      |> Option.value(~default=LanguageConfiguration.default);

    let characterSelection =
      editor |> Feature_Editor.Editor.byteRangeToCharacterRange(selection);

    let (model, outmsg) =
      Feature_LanguageSupport.update(
        ~languageConfiguration,
        ~configuration=state.configuration,
        ~maybeBuffer,
        ~maybeSelection=characterSelection,
        ~editorId,
        ~cursorLocation,
        ~client=extHostClient,
        msg,
        state.languageSupport,
      );

    let eff =
      Feature_LanguageSupport.(
        switch (outmsg) {
        | Nothing => Isolinear.Effect.none
        | ApplyCompletion({insertText, meetColumn}) =>
          Service_Vim.Effects.applyCompletion(
            ~meetColumn, ~insertText, ~toMsg=cursors =>
            Actions.Editor({
              scope: EditorScope.Editor(editorId),
              msg: CursorsChanged(cursors),
            })
          )
        | InsertSnippet({meetColumn, snippet}) =>
          // TODO: Full snippet integration!
          let insertText = Feature_Snippets.snippetToInsert(~snippet);
          Service_Vim.Effects.applyCompletion(
            ~meetColumn, ~insertText, ~toMsg=cursors =>
            Actions.Editor({
              scope: EditorScope.Editor(editorId),
              msg: CursorsChanged(cursors),
            })
          );
        | OpenFile({filePath, location}) =>
          Internal.openFileEffect(~position=location, filePath)
        | NotifySuccess(msg) => Internal.notificationEffect(~kind=Info, msg)
        | NotifyFailure(msg) => Internal.notificationEffect(~kind=Error, msg)
        | Effect(eff) =>
          eff |> Isolinear.Effect.map(msg => LanguageSupport(msg))
        }
      );

    ({...state, languageSupport: model}, eff);

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
    | OpenFile({filePath, position}) => (
        state,
        Internal.openFileEffect(~position=Some(position), filePath),
      )
    | UnhandledWindowMovement(windowMovement) => (
        state,
        Internal.unhandledWindowMotionEffect(windowMovement),
      )
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

  | Search(msg) =>
    let (model, maybeOutmsg) = Feature_Search.update(state.searchPane, msg);
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
    | Some(UnhandledWindowMovement(windowMovement)) => (
        state,
        Internal.unhandledWindowMotionEffect(windowMovement),
      )
    | None => (state, Effect.none)
    };

  | SCM(msg) =>
    let (model, maybeOutmsg) =
      Feature_SCM.update(extHostClient, state.scm, msg);
    let state = {...state, scm: model};

    switch ((maybeOutmsg: Feature_SCM.outmsg)) {
    | Focus => (FocusManager.push(Focus.SCM, state), Effect.none)
    | Effect(eff) => (state, eff |> Effect.map(msg => Actions.SCM(msg)))
    | EffectAndFocus(eff) => (
        FocusManager.push(Focus.SCM, state),
        eff |> Effect.map(msg => Actions.SCM(msg)),
      )

    | OpenFile(filePath) => (state, Internal.openFileEffect(filePath))
    | UnhandledWindowMovement(windowMovement) => (
        state,
        Internal.unhandledWindowMotionEffect(windowMovement),
      )
    | Nothing => (state, Effect.none)
    };

  | SideBar(msg) =>
    let sideBar' = Feature_SideBar.update(msg, state.sideBar);
    ({...state, sideBar: sideBar'}, Effect.none);
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
          {
            ...state',
            pane:
              Feature_Pane.toggle(
                ~pane=Feature_Pane.Diagnostics,
                state'.pane,
              ),
          },
          Effect.none,
        )

      | ClearNotifications => (
          {...state', notifications: Feature_Notification.initial},
          Effect.none,
        )
      | ToggleNotifications => (
          {
            ...state',
            pane:
              Feature_Pane.toggle(
                ~pane=Feature_Pane.Notifications,
                state'.pane,
              ),
          },
          Effect.none,
        )
      | ShowFileTypePicker =>
        let bufferId =
          state.layout
          |> Feature_Layout.activeEditor
          |> Feature_Editor.Editor.getBufferId;

        let languages =
          state.languageInfo
          |> Exthost.LanguageInfo.languages
          |> List.map(language =>
               (
                 language,
                 Oni_Core.IconTheme.getIconForLanguage(
                   state.iconTheme,
                   language,
                 ),
               )
             );
        (
          state',
          Isolinear.Effect.createWithDispatch(
            ~name="statusBar.fileTypePicker", dispatch => {
            dispatch(
              Actions.QuickmenuShow(FileTypesPicker({bufferId, languages})),
            )
          }),
        );

      | Effect(eff) => (
          state',
          eff |> Isolinear.Effect.map(msg => Actions.StatusBar(msg)),
        )
      };

    (state'', eff);

  | Buffers(Feature_Buffers.Update({update, newBuffer, _}) as msg) =>
    let buffers = Feature_Buffers.update(msg, state.buffers);

    let syntaxHighlights =
      Feature_Syntax.handleUpdate(
        ~scope=
          Internal.getScopeForBuffer(
            ~languageInfo=state.languageInfo,
            newBuffer,
          ),
        ~grammars=grammarRepository,
        ~config=Feature_Configuration.resolver(state.config, state.vim),
        ~theme=state.tokenTheme,
        update,
        state.syntaxHighlights,
      );

    let state = {...state, buffers, syntaxHighlights};

    let (state, eff) = (
      state,
      Feature_Syntax.Effect.bufferUpdate(
        ~bufferUpdate=update,
        state.syntaxHighlights,
      )
      |> Isolinear.Effect.map(() => Actions.Noop),
    );
    open Feature_Editor; // update editor

    let buffer = EditorBuffer.ofBuffer(newBuffer);
    let bufferId = Buffer.getId(newBuffer);
    (
      {
        ...state,
        layout:
          Feature_Layout.map(
            editor =>
              if (Editor.getBufferId(editor) == bufferId) {
                Editor.updateBuffer(~buffer, editor);
              } else {
                editor;
              },
            state.layout,
          ),
      },
      eff,
    );

  // TEMPORARY: Needs https://github.com/onivim/oni2/pull/1627 to remove
  | Buffers(Feature_Buffers.Entered({buffer, _}) as msg) =>
    let editorBuffer = buffer |> Feature_Editor.EditorBuffer.ofBuffer;

    let buffers = Feature_Buffers.update(msg, state.buffers);

    let config = Feature_Configuration.resolver(state.config, state.vim);
    (
      {
        ...state,
        buffers,
        layout:
          Feature_Layout.openEditor(
            ~config,
            Feature_Editor.Editor.create(~config, ~buffer=editorBuffer, ()),
            state.layout,
          ),
      },
      Effect.none,
    );

  | Buffers(msg) =>
    let buffers = Feature_Buffers.update(msg, state.buffers);
    ({...state, buffers}, Effect.none);

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
    let (config, outmsg) =
      Feature_Configuration.update(~getUserSettings, state.config, msg);
    let state = {...state, config};
    let eff =
      switch (outmsg) {
      | ConfigurationChanged({changed}) =>
        Isolinear.Effect.create(
          ~name="featuers.configuration$acceptConfigurationChanged", () => {
          let configuration =
            Feature_Configuration.toExtensionConfiguration(
              config,
              Feature_Extensions.all(state.extensions),
              setup,
            );
          let changed = Exthost.Configuration.Model.fromSettings(changed);
          Exthost.Request.Configuration.acceptConfigurationChanged(
            ~configuration,
            ~changed,
            extHostClient,
          );
        })
      | Nothing => Effect.none
      };

    (state, eff);

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

    let focus =
      switch (FocusManager.current(state)) {
      | Pane => Some(Bottom)

      | Editor
      | Terminal(_) => Some(Center)

      | Extensions
      | FileExplorer
      | SCM
      | Search => Some(Left)

      | _ => None
      };
    let (model, outmsg) = update(~focus, state.layout, msg);
    let state = {...state, layout: model};

    switch (outmsg) {
    | Focus(Center) => (FocusManager.push(Editor, state), Effect.none)

    | Focus(Left) => (
        Feature_SideBar.isOpen(state.sideBar)
          ? SideBarReducer.focus(state) : state,
        Effect.none,
      )

    | Focus(Bottom) =>
      let pane = state.pane |> Feature_Pane.selected;
      (
        {...state, pane: Feature_Pane.show(~pane, state.pane)}
        |> FocusManager.push(Pane),
        Effect.none,
      );

    | SplitAdded => ({...state, zenMode: false}, Effect.none)

    | RemoveLastWasBlocked => (state, Internal.quitEffect)

    | Nothing => (state, Effect.none)
    };

  | Terminal(msg) =>
    let (model, eff) =
      Feature_Terminal.update(
        ~config=Feature_Configuration.resolver(state.config, state.vim),
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
      | TerminalCreated({name, splitDirection}) =>
        let windowTreeDirection =
          switch (splitDirection) {
          | Horizontal => Some(`Horizontal)
          | Vertical => Some(`Vertical)
          | Current => None
          };

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

  | Theme(msg) =>
    let (model', outmsg) = Feature_Theme.update(state.colorTheme, msg);

    let eff =
      switch (outmsg) {
      | OpenThemePicker(_) =>
        let themes =
          state.extensions
          |> Feature_Extensions.pick((manifest: Exthost.Extension.Manifest.t) => {
               Exthost.Extension.Contributions.(manifest.contributes.themes)
             })
          |> List.flatten;

        Isolinear.Effect.createWithDispatch(~name="menu", dispatch => {
          dispatch(Actions.QuickmenuShow(ThemesPicker(themes)))
        });
      | Nothing => Isolinear.Effect.none
      };

    ({...state, colorTheme: model'}, eff);

  | Notification(msg) =>
    let model' = Feature_Notification.update(state.notifications, msg);
    ({...state, notifications: model'}, Effect.none);

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
      Service_OS.Effect.statMultiple(paths, (path, stats) =>
        switch (stats.st_kind) {
        | S_REG => OpenFileByPath(path, None, None)
        | S_DIR =>
          switch (Luv.Path.chdir(path)) {
          | Ok () => DirectoryChanged(path)
          | Error(_) => Noop
          }
        | _ => Noop
        }
      );
    (state, eff);

  // TODO: Merge into Editor(...) update
  | Editor({scope, msg: CursorsChanged(_) as msg}) =>
    let prevCursor =
      state.layout
      |> Feature_Layout.activeEditor
      |> Feature_Editor.Editor.getPrimaryCursor;

    let maybeBuffer = Selectors.getActiveBuffer(state);
    let editor = Feature_Layout.activeEditor(state.layout);
    let (signatureHelp, shOutMsg) =
      Feature_SignatureHelp.update(
        ~maybeBuffer,
        ~maybeEditor=Some(editor),
        ~extHostClient,
        state.signatureHelp,
        Feature_SignatureHelp.CursorMoved(
          Feature_Editor.Editor.getId(editor),
        ),
      );

    let shEffect =
      switch (shOutMsg) {
      | Effect(e) => Effect.map(msg => Actions.SignatureHelp(msg), e)
      | _ => Effect.none
      };
    let (layout, editorEffect) =
      Internal.updateEditors(~scope, ~msg, state.layout);

    let newCursor =
      layout
      |> Feature_Layout.activeEditor
      |> Feature_Editor.Editor.getPrimaryCursor;

    let languageSupport =
      if (prevCursor != newCursor) {
        Feature_LanguageSupport.cursorMoved(
          ~previous=prevCursor,
          ~current=newCursor,
          state.languageSupport,
        );
      } else {
        state.languageSupport;
      };

    let state = {...state, layout, signatureHelp, languageSupport};
    let effect = [shEffect, editorEffect] |> Effect.batch;
    (state, effect);

  | Editor({scope, msg}) =>
    let (layout, effect) =
      Internal.updateEditors(~scope, ~msg, state.layout);
    let state = {...state, layout};
    (state, effect);

  | Changelog(msg) =>
    let (model, eff) = Feature_Changelog.update(state.changelog, msg);
    ({...state, changelog: model}, eff);

  // TODO: This should live in the editor feature project
  | EditorFont(Service_Font.FontLoaded(font)) =>
    let buffers' =
      state.buffers |> IntMap.map(Oni_Core.Buffer.setFont(font));
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
                   Feature_Editor.Editor.updateBuffer(
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

  // TODO: This should live in the terminal feature project
  | TerminalFont(Service_Font.FontLoaded(font)) => (
      {...state, terminalFont: font},
      Isolinear.Effect.none,
    )
  | TerminalFont(Service_Font.FontLoadError(message)) => (
      state,
      Internal.notificationEffect(~kind=Error, message),
    )

  | TitleBar(titleBarMsg) =>
    let eff =
      switch (
        Feature_TitleBar.update(
          ~maximize,
          ~minimize,
          ~close,
          ~restore,
          titleBarMsg,
        )
      ) {
      | Feature_TitleBar.Effect(effect) => effect
      | Feature_TitleBar.Nothing => Isolinear.Effect.none
      };

    (state, eff |> Isolinear.Effect.map(msg => TitleBar(msg)));

  | SignatureHelp(msg) =>
    let maybeBuffer = Selectors.getActiveBuffer(state);
    let editor = Feature_Layout.activeEditor(state.layout);
    let (model', eff) =
      Feature_SignatureHelp.update(
        ~maybeBuffer,
        ~maybeEditor=Some(editor),
        ~extHostClient,
        state.signatureHelp,
        msg,
      );
    let effect =
      switch (eff) {
      | Feature_SignatureHelp.Nothing => Effect.none
      | Feature_SignatureHelp.Effect(eff) =>
        Effect.map(msg => Actions.SignatureHelp(msg), eff)
      | Feature_SignatureHelp.Error(str) =>
        Internal.notificationEffect(
          ~kind=Error,
          "Signature help error: " ++ str,
        )
      };
    ({...state, signatureHelp: model'}, effect);

  | ExtensionBufferUpdateQueued({triggerKey}) =>
    let maybeBuffer = Selectors.getActiveBuffer(state);
    let editor = Feature_Layout.activeEditor(state.layout);
    let activeCursor = editor |> Feature_Editor.Editor.getPrimaryCursor;
    let activeCursorByte =
      editor |> Feature_Editor.Editor.getPrimaryCursorByte;
    let (signatureHelp, shOutMsg) =
      Feature_SignatureHelp.update(
        ~maybeBuffer,
        ~maybeEditor=Some(editor),
        ~extHostClient,
        state.signatureHelp,
        Feature_SignatureHelp.KeyPressed(triggerKey, false),
      );
    let shEffect =
      switch (shOutMsg) {
      | Effect(e) => Effect.map(msg => Actions.SignatureHelp(msg), e)
      | _ => Effect.none
      };

    let languageSupport' =
      maybeBuffer
      |> Option.map(buffer => {
           let syntaxScope =
             Feature_Syntax.getSyntaxScope(
               ~bufferId=Buffer.getId(buffer),
               ~bytePosition=activeCursorByte,
               state.syntaxHighlights,
             );
           let config =
             Feature_Configuration.resolver(state.config, state.vim);
           Feature_LanguageSupport.bufferUpdated(
             ~buffer,
             ~config,
             ~activeCursor,
             ~syntaxScope,
             ~triggerKey,
             state.languageSupport,
           );
         })
      |> Option.value(~default=state.languageSupport);

    ({...state, signatureHelp, languageSupport: languageSupport'}, shEffect);

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
               Editor.setYankHighlight(
                 ~yankHighlight={
                   key: Brisk_reconciler.Key.create(),
                   pixelRanges,
                 },
                 editor,
               );
             } else {
               editor;
             }
           );
      ({...state, layout: layout'}, Isolinear.Effect.none);
    };

  | Vim(msg) =>
    let wasInInsertMode = Feature_Vim.mode(state.vim) == Vim.Mode.Insert;
    let (vim, outmsg) = Feature_Vim.update(msg, state.vim);
    let state = {...state, vim};

    let (state', eff) =
      switch (outmsg) {
      | Nothing => (state, Isolinear.Effect.none)
      | Effect(e) => (state, e)
      | CursorsUpdated(cursors) =>
        open Feature_Editor;
        let activeEditorId =
          state.layout |> Feature_Layout.activeEditor |> Editor.getId;

        let layout' =
          state.layout
          |> Feature_Layout.map(editor =>
               if (Editor.getId(editor) == activeEditorId) {
                 Editor.setCursors(~cursors, editor);
               } else {
                 editor;
               }
             );
        ({...state, layout: layout'}, Isolinear.Effect.none);
      };

    let isInInsertMode = Feature_Vim.mode(state'.vim) == Vim.Mode.Insert;

    // Entered insert mode
    let languageSupport =
      if (isInInsertMode && !wasInInsertMode) {
        state.languageSupport |> Feature_LanguageSupport.startInsertMode;
                                                                    // Exited insert mode
      } else if (!isInInsertMode && wasInInsertMode) {
        state.languageSupport |> Feature_LanguageSupport.stopInsertMode;
      } else {
        state.languageSupport;
      };

    (
      {...state', languageSupport},
      eff |> Isolinear.Effect.map(msg => Actions.Vim(msg)),
    );

  | AutoUpdate(msg) =>
    let (state', outmsg) = Feature_AutoUpdate.update(state.autoUpdate, msg);

    let eff =
      (
        switch (outmsg) {
        | Nothing => Isolinear.Effect.none
        | Effect(eff) => eff
        }
      )
      |> Isolinear.Effect.map(msg => Actions.AutoUpdate(msg));

    ({...state, autoUpdate: state'}, eff);

  | _ => (state, Effect.none)
  };

// SUBSCRIPTIONS

module QuickmenuSubscriptionRunner =
  Subscription.Runner({
    type action = Actions.t;
    let id = "quickmenu-subscription";
  });

module SearchSubscriptionRunner =
  Subscription.Runner({
    type action = Feature_Search.msg;
    let id = "search-subscription";
  });

let updateSubscriptions = (setup: Setup.t) => {
  let ripgrep = Ripgrep.make(~executablePath=setup.rgPath);

  let quickmenuSubscriptions = QuickmenuStoreConnector.subscriptions(ripgrep);

  let searchSubscriptions = Feature_Search.subscriptions(ripgrep);

  (state: State.t, dispatch) => {
    quickmenuSubscriptions(dispatch, state)
    |> QuickmenuSubscriptionRunner.run(~dispatch);

    let searchDispatch = msg => dispatch(Search(msg));
    searchSubscriptions(searchDispatch, state.searchPane)
    |> SearchSubscriptionRunner.run(~dispatch=searchDispatch);
  };
};
