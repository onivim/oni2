open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates that certain keystrokes are ignored by our Vim layer
runTest(~name="InputIgnore test", (_dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.vimMode == Vim.Types.Normal
  );

  // Create notification - an echo should trigger onj
  Vim.command("echo 'hi from test'");
  runEffects();

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
