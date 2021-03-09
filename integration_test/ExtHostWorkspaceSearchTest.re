open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates:
// - The 'oni-dev' extension gets activated
// - When typing in an 'oni-dev' buffer, we get some completion results
runTest(~name="ExtHostWorkspaceSearchTest", ({dispatch, wait, _}) => {
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

  // Search for 1 match
  dispatch(
    Actions.Extensions(
      Feature_Extensions.Msg.command(
        ~command="_test.findFiles",
        ~arguments=[`Int(1)],
      ),
    ),
  );

  // Wait for a notification
  wait(
    ~timeout=30.,
    ~name="Got success notification for 1 result",
    (state: State.t) => {
      let notifications = Feature_Notification.all(state.notifications);
      notifications
      |> List.exists((notification: Feature_Notification.notification) => {
           notification.message == "success:1"
         });
    },
  );

  // Search for multiple matches
  dispatch(
    Actions.Extensions(
      Feature_Extensions.Msg.command(
        ~command="_test.findFiles",
        ~arguments=[`Int(3)],
      ),
    ),
  );

  // Wait for a notification
  wait(
    ~timeout=60.,
    ~name="Got success notification for 5 results",
    (state: State.t) => {
      let notifications = Feature_Notification.all(state.notifications);
      notifications
      |> List.exists((notification: Feature_Notification.notification) => {
           notification.message == "success:3"
         });
    },
  );
});
