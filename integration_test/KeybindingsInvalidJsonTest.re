open Oni_Model;
open Oni_IntegrationTestLib;

let keybindings = Some({|
invalid json
|});

runTest(
  ~keybindings,
  ~name="KeybindingsInvalidJson",
  ({wait, _}) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      Selectors.mode(state) |> Vim.Mode.isNormal
    );

    let getErrorNotificationCount = (state: State.t) => {
      let notifications: list(Feature_Notification.notification) =
        Feature_Notification.all(state.notifications);
      notifications
      |> List.filter((n: Feature_Notification.notification) =>
           n.kind == Feature_Notification.Error
         )
      |> List.length;
    };

    wait(~name="Get error message for invalid keybinding", (state: State.t) =>
      getErrorNotificationCount(state) > 0
    );
  },
);
