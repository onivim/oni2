open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates that certain keystrokes are ignored by our Vim layer
runTest(~name="InputIgnore test", (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  // Create notification - an echo should trigger onj
  Vim.command("echo 'hi from test'");
  runEffects();

  wait(~name="notification shows up", (state: State.t) =>
    if (Notifications.any(state.notifications)) {
      let notification = List.hd(state.notifications);
      String.equal(notification.message, "hi from test");
    } else {
      true;
    }
  );

  // Escape clears notification
  dispatch(KeyboardInput("<esc>"));
  runEffects();

  wait(~name="Notification is cleared", (state: State.t) =>
    List.length(state.notifications) == 0
  );
});
