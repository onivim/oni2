open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module TS = TextSynchronization;

let configuration = {|
{
    "developer.oni-dev.formatter": "singleLine",
}
|};

// This test validates formatting via a large-uber-edit, like `prettier` and the ocaml extension provide
runTest(
  ~configuration=Some(configuration),
  ~name="ExtHostFormatSingleEditTest",
  ({input, dispatch, wait, _}) => {
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

    let unformattedFilePath = getAssetPath("unformatted1.oni-dev");

    // Create a buffer
    dispatch(
      Actions.OpenFileByPath(
        unformattedFilePath,
        SplitDirection.Current,
        None,
      ),
    );

    // Wait for the oni-dev filetype
    wait(
      ~timeout=30.0,
      ~name="Wait for oni-dev filetype to show up",
      (state: State.t) => {
        let fileType =
          Selectors.getActiveBuffer(state)
          |> Option.map(Buffer.getFileType)
          |> Option.map(Buffer.FileType.toString);

        fileType == Some("oni-dev");
      },
    );

    // Format document
    input("gg=G");

    TS.validateTextIsSynchronized(
      ~expectedText=Some("a|!!INSERT|b|c|!!INSERT"),
      ~description="after formatting",
      dispatch,
      wait,
    );
  },
);
