open Oni_Core;
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

  // We're going to open and close Ripgrep ~300 times.
  // The leaking-file-descriptor bug would leak ~3 fds every time we
  // run QuickOpen. Simulating running ripgrep 300 times would
  // easily hit this limit and exercise that failure condition.
  let iterations = ref(0);
  let maxIterations = 300;

  while (iterations^ < maxIterations) {
    let currentRunCount = Ripgrep.getRunCount();

    wait(
      ~name="Ripgrep iterations: round " ++ string_of_int(iterations^),
      (_state: State.t) => {
        // Changing the search would
        dispatch(MenuSearch("a"));
        runEffects();
        dispatch(MenuSearch("ab"));
        runEffects();
        print_endline(
          "Ripgrep has been opened: " ++ string_of_int(Ripgrep.getRunCount()),
        );
        Ripgrep.getRunCount() > currentRunCount;
      },
    );

    incr(iterations);
  };
});
