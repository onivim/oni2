open EditorCoreTypes;
open Oni_Model;
open Oni_IntegrationTestLib;
module Editor = Feature_Editor.Editor;

runTestWithInput(~name="RegressionFontFallback", (input, dispatch, wait, _) => {
  wait(~name="Wait for split to be created 1", (state: State.t) => {
    let splitCount =
      state.layout |> Feature_Layout.visibleEditors |> List.length;
    splitCount == 1;
  });

  // Open a file with an extensive set of UTF-8 characters
  // Integration test renders every time the state changes,
  // so we'd expect to hit a hang / crash if we hit an invalid fallback case.
  let testFile = getAssetPath("utf8-test-file.htm");

  // Open test file
  dispatch(Actions.OpenFileByPath(testFile, None, None));

  wait(~name="Wait for cursor to be ready", (state: State.t) => {
    let location: Location.t =
      state.layout |> Feature_Layout.activeEditor |> Editor.getPrimaryCursor;

    location.line == Index.zero;
  });

  input("G");

  wait(~name="Wait for cursor to be at bottom", (state: State.t) => {
    let location: Location.t =
      state.layout |> Feature_Layout.activeEditor |> Editor.getPrimaryCursor;

    location |> Location.show |> prerr_endline;
    location.line |> Index.toZeroBased |> string_of_int |> prerr_endline;

    location.line == Index.fromZeroBased(212);
  });
});
