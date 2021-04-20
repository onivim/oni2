open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module TS = TextSynchronization;

let configuration = {|
{
    "editor.formatOnSave": true
}
|};

runTest(
  ~configuration=Some(configuration),
  ~name="FormatOnSave",
  ({input, dispatch, wait, key, _}) => {
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

    let unformattedFilePath = getAssetPath("test.unformatted.js");

    // Create a buffer
    dispatch(
      Actions.OpenFileByPath(
        unformattedFilePath,
        SplitDirection.Current,
        None,
      ),
    );

    wait(
      ~timeout=30.0,
      ~name="Validate first line of buffer is unformatted",
      (state: State.t) => {
      Selectors.getActiveBuffer(state)
      |> Option.map(buffer =>
           if (Buffer.getNumberOfLines(buffer) > 0) {
             let rawLine = Buffer.getLine(0, buffer) |> BufferLine.raw;

             String.equal(rawLine, "    console.log");
           } else {
             false;
           }
         )
      |> Option.value(~default=false)
    });

    // Wait for JS extension, we need it for formatting
    wait(
      ~timeout=30.0,
      ~name=
        "Validate the 'vscode.typescript-language-features' extension gets activated",
      (state: State.t) => {
        Feature_Extensions.activatedIds(state.extensions)
        |> List.iter(id => prerr_endline(id));
        List.exists(
          id => id == "vscode.typescript-language-features",
          state.extensions |> Feature_Extensions.activatedIds,
        );
      },
    );

    // Save file
    input(":");
    input("w");
    input("!");
    key(EditorInput.Key.Return);

    wait(
      ~timeout=30.0,
      ~name="Validate first line of buffer gets formatted",
      (state: State.t) => {
      Selectors.getActiveBuffer(state)
      |> Option.map(buffer => {
           let rawLine = Buffer.getLine(0, buffer) |> BufferLine.raw;

           String.equal(rawLine, "console.log");
         })
      |> Option.value(~default=false)
    });

    TS.validateTextIsSynchronized(
      ~expectedText=Some("console.log|console.warn|console.error"),
      ~description="after formatting",
      dispatch,
      wait,
    );
  },
);
