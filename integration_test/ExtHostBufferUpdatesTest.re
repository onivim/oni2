open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module TS = TextSynchronization;

// This test validates:
// - The 'oni-dev' extension gets activated
// - When typing in an 'oni-dev' buffer, the buffer received by the extension host
// is in sync with the buffer in the main process
runTest(~name="ExtHostBufferUpdates", ({input, dispatch, wait, key, _}) => {
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

  // Create a buffer
  dispatch(Actions.OpenFileByPath("test.oni-dev", None, None));

  // Wait for the oni-dev filetype
  wait(
    ~timeout=30.0,
    ~name="Validate the 'oni-dev' extension gets activated",
    (state: State.t) => {
      let fileType =
        Selectors.getActiveBuffer(state)
        |> Option.map(Buffer.getFileType)
        |> Option.map(Buffer.FileType.toString);

      fileType == Some("oni-dev");
    },
  );

  // Enter some text
  input("i");

  input("a");
  key(EditorInput.Key.Return);

  input("b");
  key(EditorInput.Key.Return);

  input("c");
  key(EditorInput.Key.Escape);

  // TODO: Do we need to wait to ensure the buffer update gets sent?
  TS.validateTextIsSynchronized(
    ~expectedText=Some("a|b|c|"),
    ~description="after insert mode",
    dispatch,
    wait,
  );

  // Now, delete a line - we had some bugs where deletes were not sync'd properly
  input("gg");
  input("j");
  input("dd");

  TS.validateTextIsSynchronized(
    ~expectedText=Some("a|c|"),
    ~description="after deleting some lines",
    dispatch,
    wait,
  );

  // Undo the change - we also had bugs here!
  input("u");

  TS.validateTextIsSynchronized(
    ~expectedText=Some("a|b|c|"),
    ~description="after undo",
    dispatch,
    wait,
  );

  // Finally, modify a single line
  input("gg");
  input("Iabc");
  key(EditorInput.Key.Escape);

  TS.validateTextIsSynchronized(
    ~expectedText=Some("abca|b|c|"),
    ~description="after inserting some text in an existing line",
    dispatch,
    wait,
  );

  // Insert multiple lines, and then undo
  // This was a case discovered while investigating #2196
  input("gg");
  input("O");
  key(EditorInput.Key.Return);
  key(EditorInput.Key.Return);
  key(EditorInput.Key.Escape);

  TS.validateTextIsSynchronized(
    ~expectedText=Some("|||abca|b|c|"),
    ~description="after inserting multiple lines",
    dispatch,
    wait,
  );

  // Undo the change - we also had bugs here!
  input("u");
  TS.validateTextIsSynchronized(
    ~expectedText=Some("abca|b|c|"),
    ~description="after inserting multiple lines",
    dispatch,
    wait,
  );
});
