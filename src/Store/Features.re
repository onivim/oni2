open Isolinear;
open Oni_Core;
open Oni_Model;
open Actions;

// UPDATE

let update =
    (
      ~extHostClient,
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

  | BufferUpdate({update, _}) =>
    let syntaxHighlights =
      Feature_Syntax.update(
        state.syntaxHighlights,
        Feature_Syntax.BufferUpdated(update),
      );
    let state = {...state, syntaxHighlights};
    (state, Effect.none);

  | Configuration(msg) =>
    let (config, outmsg) =
      Feature_Configuration.update(~getUserSettings, state.config, msg);
    let state = {...state, config};
    let eff =
      switch (outmsg) {
      | ConfigurationChanged({changed}) =>
        Oni_Extensions.ExtHostClient.Effects.acceptConfigurationChanged(
          extHostClient,
          Feature_Configuration.toExtensionConfiguration(
            config,
            state.extensions.extensions,
            setup,
          ),
          ~changed=Oni_Extensions.Configuration.Model.fromSettings(changed),
        )
      | Nothing => Effect.none
      };

    (state, eff);

  | Syntax(msg) =>
    let syntaxHighlights = Feature_Syntax.update(state.syntaxHighlights, msg);
    let state = {...state, syntaxHighlights};
    (state, Effect.none);

  | Terminal(msg) =>
    let (model, eff) = Feature_Terminal.update(state.terminals, msg);

    let effect: Isolinear.Effect.t(Actions.t) =
      switch ((eff: Feature_Terminal.outmsg)) {
      | Nothing => Effect.none
      | Effect(eff) => eff |> Effect.map(msg => Actions.Terminal(msg))
      | TerminalCreated({name, splitDirection}) =>
        let windowTreeDirection =
          switch (splitDirection) {
          | Horizontal => Some(WindowTree.Horizontal)
          | Vertical => Some(WindowTree.Vertical)
          | Current => None
          };

        Isolinear.Effect.createWithDispatch(
          ~name="feature.terminal.openBuffer", dispatch => {
          dispatch(Actions.OpenFileByPath(name, windowTreeDirection, None))
        });
      };
    ({...state, terminals: model}, effect);

  | Theme(msg) =>
    let model' = Feature_Theme.update(state.colorTheme, msg);
    ({...state, colorTheme: model'}, Effect.none);

  | Notification(msg) =>
    let model' = Feature_Notification.update(state.notifications, msg);
    ({...state, notifications: model'}, Effect.none);

  | Modal(msg) =>
    switch (state.modal) {
    | Some(model) =>
      let (model', eff) = Oni_UI.Modals.update(model, msg);
      ({...state, modal: Some(model')}, eff);

    | None => (state, Effect.none)
    }

  | NotifyKeyPressed(time, key) =>
    switch (state.keyDisplayer) {
    | Some(model) when Oni_Input.Filter.filter(key) => (
        {
          ...state,
          keyDisplayer:
            Some(Oni_Components.KeyDisplayer.add(~time, key, model)),
        },
        Effect.none,
      )

    | _ => (state, Effect.none)
    }

  // TODO: This should live in the editor feature project
  | EditorFont(Service_Font.FontLoaded(font)) => (
      {...state, editorFont: font},
      Isolinear.Effect.none,
    )
  | EditorFont(Service_Font.FontLoadError(message)) => (
      state,
      Feature_Notification.Effects.create(~kind=Error, message)
      |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
    )

  // TODO: This should live in the terminal feature project
  | TerminalFont(Service_Font.FontLoaded(font)) => (
      {...state, terminalFont: font},
      Isolinear.Effect.none,
    )
  | TerminalFont(Service_Font.FontLoadError(message)) => (
      state,
      Feature_Notification.Effects.create(~kind=Error, message)
      |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
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
