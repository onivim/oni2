open Isolinear;
open Oni_Core;
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
         Oni_Extensions.LanguageInfo.getScopeFromLanguage(
           languageInfo,
           fileType,
         )
       )
    |> Option.value(~default="source.plaintext");
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
    (
      state,
      Feature_Syntax.Effect.bufferUpdate(
        ~bufferUpdate=update,
        state.syntaxHighlights,
      )
      |> Isolinear.Effect.map(() => Actions.Noop),
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
              state.extensions.extensions,
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
      EditorGroups.getActiveEditorGroup(state.editorGroups)
      |> Option.map((group: EditorGroup.t) => group.editorGroupId);
    let (model, maybeOutmsg) = update(~focus, state.layout, msg);
    let state = {...state, layout: model};

    let state =
      switch (maybeOutmsg) {
      | Focus(editorGroupId) => {
          ...state,
          editorGroups:
            EditorGroups.setActiveEditorGroup(
              editorGroupId,
              state.editorGroups,
            ),
        }
      | Nothing => state
      };
    (state, Effect.none);

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
        let maybeTerminalBuffer =
          state |> Selectors.getBufferForTerminal(~terminalId);

        // TODO:
        // This is really duplicated logic from the WindowsStoreConnector
        // - the fact that the window layout needs to be adjusted along
        // with the editor groups. We need to consolidate this to a
        // unified concept, once the window layout work has completed:
        // Something like `Feature_EditorLayout`, which contains
        // both the editor groups and layout concepts (dependent on
        // `Feature_Layout`) - and could include the `WindowsStoreConnector`.

        let editorGroups' =
          maybeTerminalBuffer
          |> Option.map(bufferId =>
               EditorGroups.closeBuffer(~bufferId, state.editorGroups)
             )
          |> Option.value(~default=state.editorGroups);

        let layout' =
          state.layout
          |> Feature_Layout.windows
          |> List.fold_left(
               (acc, editorGroupId) =>
                 if (Oni_Model.EditorGroups.getEditorGroupById(
                       editorGroups',
                       editorGroupId,
                     )
                     == None) {
                   Feature_Layout.removeWindow(editorGroupId, acc);
                 } else {
                   acc;
                 },
               state.layout,
             );

        let state' = {...state, layout: layout', editorGroups: editorGroups'};

        (state', Effect.none);
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
  | Editor({editorId, msg}) =>
    let (editorGroups', effects) =
      EditorGroups.updateEditor(~editorId, msg, state.editorGroups);

    let effect =
      effects
      |> List.map(
           fun
           | Feature_Editor.Nothing => Effect.none
           | Feature_Editor.MouseHovered(location) =>
             Effect.createWithDispatch(~name="editor.mousehovered", dispatch => {
               dispatch(Hover(Feature_Hover.MouseHovered(location)))
             })
           | Feature_Editor.MouseMoved(location) =>
             Effect.createWithDispatch(~name="editor.mousemoved", dispatch => {
               dispatch(Hover(Feature_Hover.MouseMoved(location)))
             }),
         )
      |> Isolinear.Effect.batch;

    ({...state, editorGroups: editorGroups'}, effect);
  | Changelog(msg) =>
    let (model, eff) = Feature_Changelog.update(state.changelog, msg);
    ({...state, changelog: model}, eff);

  // TODO: This should live in the editor feature project
  | EditorFont(Service_Font.FontLoaded(font)) => (
      {...state, editorFont: font},
      Isolinear.Effect.none,
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
    let maybeEditor =
      state |> Selectors.getActiveEditorGroup |> Selectors.getActiveEditor;
    let (model', eff) =
      Feature_Hover.update(
        ~maybeBuffer,
        ~maybeEditor,
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
