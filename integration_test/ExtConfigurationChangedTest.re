open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates:
// - The 'oni-dev' extension gets activated
// - That changes to configuration settings are properly propagated to extensions
// - That extension messages are delivered as notifications Oni-side
runTestWithInput(
  ~name="ExtConfigurationChangedTest", (_input, dispatch, wait, _runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    state.notifications == []
  );

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

  // Change setting
  setUserSettings(
    Config.Settings.fromList([
      ("developer.oni.test", Json.Encode.string("42")),
    ]),
  );
  dispatch(Actions.Configuration(UserSettingsChanged));

  // Should get completions
  wait(
    ~timeout=10.0,
    ~name="Validate we get message from the 'oni-dev' extension",
    (state: State.t) =>
    switch (state.notifications) {
    | [{message, _}, _] => message == "Setting changed: 42"
    | _ => false
    }
  );
});
