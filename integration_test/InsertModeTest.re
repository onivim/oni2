open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="InsertMode test", (dispatch, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Feature_Vim.mode(state.vim) |> Vim.Mode.isNormal
  );

  dispatch(KeyboardInput({isText: true, input: "i"}));

  wait(~name="Mode switches to insert", (state: State.t) =>
    Feature_Vim.mode(state.vim) |> Vim.Mode.isInsert
  );
});
