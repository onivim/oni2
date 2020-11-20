open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="EchoNotificationTest", (dispatch, wait, _runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  dispatch(VimExecuteCommand("echo 'hi from test'"));

  wait(~name="notification shows up", (state: State.t) => {
    let notifications = Feature_Notification.all(state.notifications);
    if (notifications != []) {
      let notification = List.hd(notifications);
      String.equal(notification.message, "hi from test");
    } else {
      true;
    };
  });
});
