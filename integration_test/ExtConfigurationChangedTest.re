open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates:
// - The 'oni-dev' extension gets activated
// - That changes to configuration settings are properly propagated to extensions
// - That extension messages are delivered as notifications Oni-side
runTest(~name="ExtConfigurationChangedTest", ({dispatch, wait, _}) => {
  wait(~name="Capture initial state", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
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

  dispatch(
    Actions.Configuration(
      Feature_Configuration.Testing.transform(
        ConfigurationTransformer.setField(
          "developer.oni.test",
          `String("42"),
        ),
      ),
    ),
  );

  // Should get completions
  wait(
    ~timeout=10.0,
    ~name="Validate we get message from the 'oni-dev' extension",
    (state: State.t) => {
    switch (Feature_Notification.all(state.notifications)) {
    | [{message, _}, ..._] => String.equal(message, "Setting changed: 42")
    | _ => false
    }
  });
});
