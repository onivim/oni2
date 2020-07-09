open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="InsertMode test", (dispatch, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Types.Normal
  );

  dispatch(KeyboardInput("i"));

  wait(~name="Mode switches to insert", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Types.Insert
  );
});
