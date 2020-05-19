open TestFramework;

open Oni_Core;
open Exthost;

module TestExtensionsEvent = {
  type t = {
    name: string,
  };

  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          name: field.required("name", string),
        }
      )
    );
  };
};

let waitForExtensionsEvent = (~name, f, context) => {
  context
  |> Test.waitForMessage(
       ~name,
       fun
       | Msg.MessageService(ShowMessage({message, _})) => {
           message
           |> Yojson.Safe.from_string
           |> Json.Decode.decode_value(TestExtensionsEvent.decode)
           |> Result.map(f)
           |> Result.value(~default=false);
         }
       | _ => false,
     );
};

// Test cases for the vscode extesnsions API:
// https://code.visualstudio.com/api/references/vscode-api#extensions
describe("ExtensionsTest", ({test, _}) => {
  test("change workspaces", ({expect, _}) => {
    Test.startWithExtensions(["oni-extensions"])
    |> Test.waitForExtensionActivation("outrunlabs.oni-extensions")
    |> Test.withClient(
         Request.Commands.executeContributedCommand(
           ~arguments=[`String("outrunlabs.oni-extensions")],
           ~command="testExtensions.getPackageJSON",
         ),
       )
    |> waitForExtensionsEvent(~name="Extensions PackageJSON", (json) => {
       expect.string(json.name).toEqual("oni-extensions");
       true;
    })
    |> Test.terminate
    |> Test.waitForProcessClosed
  })
});
