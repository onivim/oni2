open Oni_IntegrationTestLib;

open WhenExpr;
module Model = Oni_Model;
module State = Model.State;

runTest(~name="InputContextKeys", (dispatch, wait, _) => {
  // Wait until the extension is activated
  // Give some time for the exthost to start
  wait(
    ~timeout=30.0,
    ~name="Validate the 'oni-dev' extension gets activated",
    (state: State.t) =>
    List.exists(
      id => id == "oni-dev-extension",
      state.extensions |> Feature_Extensions.activatedIds,
    )
  );

  // First round of context key verification - verify we're in an editor!
  wait(
    ~name=
      "Validate editor has 'editorTextFocus' and 'normalMode' context keys",
    (state: State.t) => {
    let contextKeys = Model.ContextKeys.all(state);

    Value.asBool(ContextKeys.getValue(contextKeys, "editorTextFocus"))
    && Value.asBool(ContextKeys.getValue(contextKeys, "normalMode"));
  });

  // Create a terminal - verify we have new context keys
  dispatch(VimExecuteCommand({allowAnimation: true, command: ":term"}));

  wait(
    ~name=
      "Validate terminal has 'editorTextFocus' and 'normalMode' context keys",
    (state: State.t) => {
    let contextKeys = Model.ContextKeys.all(state);

    Value.asBool(ContextKeys.getValue(contextKeys, "terminalFocus"))
    && Value.asBool(ContextKeys.getValue(contextKeys, "insertMode"));
  });
});
