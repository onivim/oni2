open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="EchoNotificationTest", (dispatch, wait, _runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Types.Normal
  );

  dispatch(VimExecuteCommand("echo 'hi from test'"));

  wait(~name="notification shows up", (state: State.t) => {
    let notifications = (
      state.notifications :> list(Feature_Notification.notification)
    );
    if (notifications != []) {
      let notification = List.hd(notifications);
      String.equal(notification.message, "hi from test");
    } else {
      true;
    };
  });
});
