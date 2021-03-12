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
  ~name="FormatOnSave", ({input, dispatch, wait, key, _}) => {
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
  dispatch(Actions.OpenFileByPath(unformattedFilePath, None, None));

  // Save file
  input(":");
  input("w");
  input("!");
  key(EditorInput.Key.Return);

  TS.validateTextIsSynchronized(
    ~expectedText=Some("console.log|console.warn|console.error|"),
    ~description="after formatting",
    dispatch,
    wait,
  );
});
