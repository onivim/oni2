open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates:
// - The 'oni-dev' extension gets activated
// - When typing in an 'oni-dev' buffer, we get some completion results
runTest(~name="ExtHostWorkspaceSearchTest", ({input, dispatch, wait, _}) => {
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
           prerr_endline("Messages: " ++ notification.message);
           notification.message == "success:1";
         });
    },
  );

  // Search for multiple matches
  dispatch(
    Actions.Extensions(
      Feature_Extensions.Msg.command(
        ~command="_test.findFiles",
        ~arguments=[`Int(5)],
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
           prerr_endline("Messages: " ++ notification.message);
           notification.message == "success:5";
         });
    },
  );
  // Wait for the oni-dev filetype
  // wait(
  //   ~timeout=30.0,
  //   ~name="Wait for oni-dev filetype to show up",
  //   (state: State.t) => {
  //     let fileType =
  //       Selectors.getActiveBuffer(state)
  //       |> Option.map(Buffer.getFileType)
  //       |> Option.map(Buffer.FileType.toString);
  //     fileType == Some("oni-dev");
  //   },
  // );
  // Enter some text
  // input("i");
  // input("R");
  // Should get completions
  // wait(
  //   ~timeout=30.0,
  //   ~name="Validate we get some completions from the 'oni-dev' extension",
  //   (state: State.t) =>
  //   state.languageSupport
  //   |> Feature_LanguageSupport.Completion.availableCompletionCount > 0
  // );
});
