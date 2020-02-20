open Isolinear;
open Oni_Core;
open Oni_Model;
open Actions;

// UPDATE

let update = (extHostClient, state: State.t, action: Actions.t) =>
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

  | Terminal(msg) =>
    let (model, eff) =
      Feature_Terminal.update(extHostClient, state.terminals, msg);
    ({...state, terminals: model}, eff);

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
