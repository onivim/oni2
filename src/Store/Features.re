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

  let getScopeForBuffer = (~languageInfo, buffer: Oni_Core.Buffer.t) => {
    buffer
    |> Oni_Core.Buffer.getFileType
    |> Utility.OptionEx.flatMap(fileType =>
         Exthost.LanguageInfo.getScopeFromLanguage(languageInfo, fileType)
       )
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
        | MouseHovered(location) =>
          Effect.createWithDispatch(~name="editor.mousehovered", dispatch => {
            dispatch(Hover(Feature_Hover.MouseHovered(location)))
          })
        | MouseMoved(location) =>
          Effect.createWithDispatch(~name="editor.mousemoved", dispatch => {
            dispatch(Hover(Feature_Hover.MouseMoved(location)))
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
      state: State.t,
      action: Actions.t,
    ) =>
  switch (action) {
  | Extensions(msg) =>
    let (model, outMsg) =
      Feature_Extensions.update(~extHostClient, msg, state.extensions);
    let state' = {...state, extensions: model};
    let effect =
      switch (outMsg) {
      | Feature_Extensions.Nothing => Effect.none
      | Feature_Extensions.Effect(eff) =>
        eff |> Isolinear.Effect.map(msg => Actions.Extensions(msg))
      };
    (state', effect);
  | Formatting(msg) =>
    let maybeBuffer = Oni_Model.Selectors.getActiveBuffer(state);
    let selection =
      state.layout
      |> Feature_Layout.activeEditor
      |> Feature_Editor.Editor.selectionOrCursorRange;

    let languageConfiguration =
      maybeBuffer
      |> OptionEx.flatMap(Oni_Core.Buffer.getFileType)
      |> OptionEx.flatMap(
           Exthost.LanguageInfo.getLanguageConfiguration(state.languageInfo),
         )
      |> Option.value(~default=LanguageConfiguration.default);

    let (model', eff) =
      Feature_Formatting.update(
        ~languageConfiguration,
        ~configuration=state.configuration,
        ~maybeBuffer,
        ~maybeSelection=Some(selection),
        ~extHostClient,
        state.formatting,
        msg,
      );
    let state' = {...state, formatting: model'};
    let effect =
      switch (eff) {
      | Feature_Formatting.Nothing => Effect.none
      | Feature_Formatting.FormattingApplied({editCount, _}) =>
        let msg = Printf.sprintf("Applied %d edits", editCount);
        Internal.notificationEffect(~kind=Info, "Format: " ++ msg);
      | Feature_Formatting.FormatError(msg) =>
        Internal.notificationEffect(~kind=Error, "Format: " ++ msg)
      | Feature_Formatting.Effect(eff) =>
        eff |> Effect.map(msg => Actions.Formatting(msg))
      };
    (state', effect);

  | Search(msg) =>
    let (model, maybeOutmsg) = Feature_Search.update(state.searchPane, msg);
    let state = {...state, searchPane: model};

    let state =
      switch (maybeOutmsg) {
      | Some(Feature_Search.Focus) => FocusManager.push(Focus.Search, state)
      | None => state
      };

    (state, Effect.none);

  | SCM(msg) =>
    let (model, maybeOutmsg) =
      Feature_SCM.update(extHostClient, state.scm, msg);
    let state = {...state, scm: model};

    let (state, eff) =
      switch ((maybeOutmsg: Feature_SCM.outmsg)) {
      | Focus => (FocusManager.push(Focus.SCM, state), Effect.none)
      | Effect(eff) => (FocusManager.push(Focus.SCM, state), eff)
      | Nothing => (state, Effect.none)
      };

    (state, eff |> Effect.map(msg => Actions.SCM(msg)));

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
    let (statusBar', maybeOutmsg) =
      Feature_StatusBar.update(state.statusBar, msg);

    let state' = {...state, statusBar: statusBar'};

    let eff =
      switch ((maybeOutmsg: Feature_StatusBar.outmsg)) {
      | Nothing => Effect.none
      };

    (state', eff);

  // TEMPORARY: Needs https://github.com/onivim/oni2/pull/1627 to remove
  | BufferEnter({buffer, _}) =>
    let editorBuffer = buffer |> Feature_Editor.EditorBuffer.ofBuffer;

    let config = Feature_Configuration.resolver(state.config);
    (
      {
        ...state,
        layout:
          Feature_Layout.openEditor(
            ~config,
            Feature_Editor.Editor.create(
              ~config,
              ~font=state.editorFont,
              ~buffer=editorBuffer,
              (),
            ),
            state.layout,
          ),
      },
      Effect.none,
    );

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

  | BufferUpdate({update, newBuffer, _}) =>
    let syntaxHighlights =
      Feature_Syntax.handleUpdate(
        ~scope=
          Internal.getScopeForBuffer(
            ~languageInfo=state.languageInfo,
            newBuffer,
          ),
        ~grammars=grammarRepository,
        ~config=Feature_Configuration.resolver(state.config),
        ~theme=state.tokenTheme,
        update,
        state.syntaxHighlights,
      );

    let state = {...state, syntaxHighlights};

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
      | Editor
      | Terminal(_) => Some(Center)

      | FileExplorer
      | SCM => Some(Left)

      | Search => Some(Bottom)

      | _ => None
      };
    let (model, outmsg) = update(~focus, state.layout, msg);
    let state = {...state, layout: model};

    switch (outmsg) {
    | Focus(Center) => (FocusManager.push(Editor, state), Effect.none)

    | Focus(Left) => (
        state.sideBar.isOpen ? SideBarReducer.focus(state) : state,
        Effect.none,
      )

    | Focus(Bottom) => (
        state.pane.isOpen ? PaneStore.focus(state) : state,
        Effect.none,
      )

    | SplitAdded => ({...state, zenMode: false}, Effect.none)

    | RemoveLastWasBlocked => (state, Internal.quitEffect)

    | Nothing => (state, Effect.none)
    };

  | Terminal(msg) =>
    let (model, eff) =
      Feature_Terminal.update(
        ~config=Feature_Configuration.resolver(state.config),
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
    let model' = Feature_Theme.update(state.colorTheme, msg);
    ({...state, colorTheme: model'}, Effect.none);

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
        if (stats.st_kind == S_REG) {
          OpenFileByPath(path, None, None);
        } else {
          Noop;
        }
      );
    (state, eff);

  | Editor({scope, msg}) =>
    let (layout, effect) =
      Internal.updateEditors(~scope, ~msg, state.layout);
    let state = {...state, layout};
    (state, effect);

  | Changelog(msg) =>
    let (model, eff) = Feature_Changelog.update(state.changelog, msg);
    ({...state, changelog: model}, eff);

  // TODO: This should live in the editor feature project
  | EditorFont(Service_Font.FontLoaded(font)) => (
      {
        ...state,
        editorFont: font,
        layout:
          Feature_Layout.map(
            editor => Feature_Editor.Editor.setFont(~font, editor),
            state.layout,
          ),
      },
      Effect.none,
    )
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

  | Hover(msg) =>
    let maybeBuffer = Oni_Model.Selectors.getActiveBuffer(state);
    let editor = Feature_Layout.activeEditor(state.layout);
    let (model', eff) =
      Feature_Hover.update(
        ~maybeBuffer,
        ~maybeEditor=Some(editor),
        ~extHostClient,
        state.hover,
        msg,
      );
    let effect =
      switch (eff) {
      | Feature_Hover.Nothing => Effect.none
      | Feature_Hover.Effect(eff) =>
        Effect.map(msg => Actions.Hover(msg), eff)
      };
    ({...state, hover: model'}, effect);

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

  | ExtensionBufferUpdateQueued(buffer) /* {triggerKey}*/ =>
    let maybeBuffer = Selectors.getActiveBuffer(state);
    let editor = Feature_Layout.activeEditor(state.layout);
    let (signatureHelp, shOutMsg) =
      Feature_SignatureHelp.update(
        ~maybeBuffer,
        ~maybeEditor=Some(editor),
        ~extHostClient,
        state.signatureHelp,
        Feature_SignatureHelp.KeyPressed(buffer.triggerKey, false),
      );
    let shEffect =
      switch (shOutMsg) {
      | Effect(e) => Effect.map(msg => Actions.SignatureHelp(msg), e)
      | _ => Effect.none
      };
    let effect = [shEffect] |> Effect.batch;
    ({...state, signatureHelp}, effect);

  | Vim(msg) => (
      {...state, vim: Feature_Vim.update(msg, state.vim)},
      Effect.none,
    )

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
