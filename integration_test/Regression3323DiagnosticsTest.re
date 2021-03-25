open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module TS = TextSynchronization;

// Validate that deleting lines with diagnostics does not crash
// Regression test for: https://github.com/onivim/oni2/issues/3323
runTest(
  ~name=
    "Regression3323 - Deleting diagnostics at top of file should not crash",
  ({input, dispatch, wait, staysTrue, _}) => {
    wait(~timeout=30.0, ~name="Exthost is initialized", (state: State.t) =>
      Feature_Exthost.isInitialized(state.exthost)
    );

    // Wait until the extension is activated
    // Give some time for the exthost to start
    wait(
      ~timeout=30.0,
      ~name="Validate the 'oni-dev' extension gets activated",
      (state: State.t) =>
      List.exists(
        id => id == "oni-dev-extension",
        state.extensions |> Feature_Extensions.activatedIds,
      )
    );

    let testFilePath = getAssetPath("some-test-file.txt");

    // Create a buffer
    dispatch(
      Actions.OpenFileByPath(testFilePath, SplitDirection.Current, None),
    );

    wait(~timeout=30.0, ~name="Validate buffer is loaded", (state: State.t) => {
      Selectors.getActiveBuffer(state)
      |> Utility.OptionEx.flatMap(buffer => {
           buffer
           |> Buffer.rawLine(EditorCoreTypes.LineNumber.zero)
           |> Option.map(String.equal("abc"))
         })
      |> Option.value(~default=false)
    });

    // Set diagnostics from extension host
    dispatch(
      Actions.Extensions(
        Feature_Extensions.Msg.command(
          ~command="developer.oni.diagnostics-add",
          ~arguments=[],
        ),
      ),
    );

    wait(
      ~timeout=30.0,
      ~name="Diagnostics get set",
      (state: State.t) => {
        let count =
          state
          |> Selectors.getActiveBuffer
          |> Option.map(
               Feature_Diagnostics.getDiagnostics(state.diagnostics),
             )
          |> Option.map(List.length)
          |> Option.value(~default=0);

        count > 0;
      },
    );

    // Delete all lines
    input("ggdG");

    staysTrue(~name="Validate survives multiple renders", ~timeout=5., _state =>
      true
    );
    // With #3323 - the rendering will crash before making it here...
    // Once formatting fix is in  - update this:
    // Validate buffer gets updated everywhere
    // TS.validateTextIsSynchronized(
    //   ~expectedText=Some(""),
    //   ~description="after formatting",
    //   dispatch,
    //   wait,
    // );
  },
);
