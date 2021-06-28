open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates:
// - The 'oni-dev' extension gets activated
// - When typing in an 'oni-dev' buffer, we get some completion results
runTest(~name="ExtHostContextTest", ({dispatch, wait, _}) => {
  wait(~timeout=30.0, ~name="Exthost is initialized", (state: State.t) =>
    Feature_Exthost.isInitialized(state.exthost)
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

  // Kick off an oni-test command that should activate the oni-test-extension:
  dispatch(
    Actions.Extensions(
      Feature_Extensions.Msg.command(
        ~command="oni-test.exthost.runFileSystemTests",
        ~arguments=[],
      ),
    ),
  );

  // Validate test passes
  wait(
    ~timeout=30.0,
    ~name="Validate test passes",
    (state: State.t) => {
      let notifications = Feature_Notification.all(state.notifications);
      notifications
      |> List.exists((notification: Feature_Notification.notification) => {
           notification.message == "PASS"
         });
    },
  );
});
