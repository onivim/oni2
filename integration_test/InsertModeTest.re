open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="InsertMode test", (dispatch, wait) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  dispatch(KeyboardInput("i"));

  wait(~name="Mode switches to insert", (state: State.t) =>
    state.mode == Vim.Types.Insert
  );
});
