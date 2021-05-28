open Oni_Model;
open Oni_IntegrationTestLib;

let configuration = {|
{
    "experimental.viml": ["set smartcase", "set ignorecase", "echo 'hello-world'"]
}
|};

runTest(
  ~configuration=Some(configuration),
  ~name="VimExperimentalVimlTest",
  ({wait, staysTrue, _}) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      Selectors.mode(state) |> Vim.Mode.isNormal
    );

    wait(~name="notification shows up", (state: State.t) => {
      let notifications = Feature_Notification.all(state.notifications);

      notifications
      |> List.exists(notification =>
           Feature_Notification.(notification.message == "hello-world")
         );
    });

    // Regression test for #3196 - verify we don't keep executing the `experimental.viml` commands:
    staysTrue(
      ~timeout=5.0,
      ~name="additional notifications do not show up",
      (state: State.t) => {
        let notifications = Feature_Notification.all(state.notifications);

        let helloNotifications =
          notifications
          |> List.filter(notification =>
               Feature_Notification.(notification.message == "hello-world")
             );
        List.length(helloNotifications) == 1;
      },
    );
  },
);
