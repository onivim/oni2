open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="QuickOpen eventually completes", (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.vimMode == Vim.Types.Normal
  );

  /* Switch to root directory */
  if (Revery.Environment.os == Revery.Environment.Mac) {
    // CI machines timeout with '/' - so we'll use home (which also reproduces the crash)
    Vim.command("cd ~") |> ignore;
  } else {
    Vim.command("cd /") |> ignore;
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
