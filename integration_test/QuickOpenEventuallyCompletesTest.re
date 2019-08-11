open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="QuickOpen eventually completes", (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  /* Switch to root directory */
  Vim.command("cd /");

  /* Launch quick open */
  dispatch(QuickOpen);
  runEffects();

  /* Wait for menu 'isLoading' to be true */
  wait(~name="Menu is loading is true", (state: State.t) =>
    state.menu.isLoading == true
  );

  let longWaitTime = 10. *. 60.; /* 10 minutes */

  /* Wait for menu 'isLoading' to eventually be false - this means quickopen completed w/o crashing */
  wait(
    ~name="Menu is loading is false",
    ~timeout=longWaitTime,
    (state: State.t) => {
      dispatch(Tick);
      runEffects();
      print_endline(
        "Current job state: \n" ++ Core.Job.show(state.menu.filterJob),
      );
      state.menu.isLoading == false;
    },
  );
});
