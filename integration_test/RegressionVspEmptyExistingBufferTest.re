open Oni_Model;
open Oni_IntegrationTestLib;
module Editor = Feature_Editor.Editor;

runTest(~name="RegressionVspEmpty", (dispatch, wait, _) => {
  let originalBuffer = ref(None);
  wait(~name="Wait for split to be created 1", (state: State.t) => {
    let splitCount =
      state.layout |> Feature_Layout.visibleEditors |> List.length;
    originalBuffer :=
      Some(state.layout |> Feature_Layout.activeEditor |> Editor.getBufferId);
    splitCount == 1;
  });

  dispatch(
    Actions.VimExecuteCommand({allowAnimation: true, command: "e test.txt"}),
  );
  wait(~name="Wait for file to get editor", (state: State.t) => {
    let currentBuffer =
      state.layout |> Feature_Layout.activeEditor |> Editor.getBufferId;

    Some(currentBuffer) != originalBuffer^;
  });

  /* :vsp with no arguments should create a second split w/ same buffer */
  dispatch(
    Actions.VimExecuteCommand({allowAnimation: true, command: "vsp"}),
  );

  wait(~name="Wait for split to be created", (state: State.t) => {
    let splitCount =
      state.layout |> Feature_Layout.visibleEditors |> List.length;

    splitCount == 2;
  });

  /* Validate the editors all have same buffer id */
  wait(~name="Wait for split to be created", (state: State.t) => {
    switch (Feature_Layout.visibleEditors(state.layout)) {
    | [e1, e2] =>
      let bufferId1 = Editor.getBufferId(e1);
      let bufferId2 = Editor.getBufferId(e2);
      print_endline("e1 buffer id: " ++ string_of_int(bufferId1));
      print_endline("e2 buffer id: " ++ string_of_int(bufferId2));
      bufferId1 == bufferId2;
    | _ => false
    }
  });
});
