open Oni_Model;
open Oni_IntegrationTestLib;

runTest(
  ~name="QuickOpen eventually completes", ({dispatch, wait, runEffects, _}) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  /* Switch to root directory */
  let (_: Vim.Context.t, _: list(Vim.Effect.t)) =
    if (Revery.Environment.isMac) {
      // CI machines timeout with '/' - so we'll use home (which also reproduces the crash)
      Vim.command(
        "cd ~",
      );
    } else {
      Vim.command("cd /");
    };

  /* Launch quick open */
  dispatch(QuickmenuShow(FilesPicker));
  runEffects();

  /* Wait for quickmenu 'isLoading' to be true */
  wait(~name="Quickmenu is open", (state: State.t) =>
    switch (state.quickmenu) {
    | Some(_) => true
    | _ => false
    }
  );

  let longWaitTime = 10. *. 60.; /* 10 minutes */

  /* Wait for quickmenu 'isLoading' to eventually be false - this means quickopen completed w/o crashing */
  wait(
    ~name="Quickmenu is loading is false",
    ~timeout=longWaitTime,
    (state: State.t) => {
    switch (state.quickmenu) {
    | Some({filterProgress: Complete, _}) => true
    | _ => false
    }
  });
});
