open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;
module Buffer = Oni_Core.Buffer;

// Set per-filetype configuration for Makefiles, and verify it is set as expected
let configuration = {|
{
    "editor.insertSpaces": true,
    "editor.tabSize": 2,
    "editor.indentSize": 2,
    "[javascript]": {
        "editor.detectIndentation": false,
        "editor.insertSpaces": false,
        "editor.tabSize": 8,
        "editor.indentSize": 8,
    }
}
|};

runTest(
  ~configuration=Some(configuration),
  ~name="ConfigurationPerFileType",
  ({dispatch, wait, _}) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      Selectors.mode(state) |> Vim.Mode.isNormal
    );

    let waitForBuffer = (~name, f) =>
      wait(
        ~name,
        ~timeout=10.,
        (state: State.t) => {
          let maybeBuffer = state |> Selectors.getActiveBuffer;

          maybeBuffer |> Option.map(f) |> Option.value(~default=false);
        },
      );

    // Create a plaintext file
    dispatch(
      Actions.OpenFileByPath("test.txt", SplitDirection.Current, None),
    );

    // Verify plaintext buffer matches the default settings:
    waitForBuffer(~name="Plaintext buffer should pick up defaults", buffer => {
      prerr_endline(
        IndentationSettings.show(Buffer.getIndentation(buffer)),
      );
      buffer
      |> Buffer.getIndentation
      == IndentationSettings.{mode: Spaces, size: 2, tabSize: 2};
    });

    // Open a new buffer, that should pick up the per-filetype settings
    dispatch(
      Actions.OpenFileByPath("test.js", SplitDirection.Current, None),
    );

    waitForBuffer(
      ~name="Wait for active buffer to pick up javascript filetype", buffer => {
      "javascript"
      == (buffer |> Buffer.getFileType |> Buffer.FileType.toString)
    });

    waitForBuffer(
      ~name="Verify javascript buffer picked up per-javascript configuration",
      buffer => {
      buffer
      |> Buffer.getIndentation
      == IndentationSettings.{mode: Tabs, size: 8, tabSize: 8}
    });
  },
);
