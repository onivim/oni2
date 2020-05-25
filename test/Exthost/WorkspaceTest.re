open TestFramework;

open Oni_Core;
open Exthost;

let workspace1Uri = Uri.fromPath("/test/workspace1");
let workspace2Uri = Uri.fromPath("/test/workspace2");

let expectedPath1 = Sys.win32 ? "\\test\\workspace1" : "/test/workspace1";
let expectedPath2 = Sys.win32 ? "\\test\\workspace2" : "/test/workspace2";

let toWorkspaceData = (~name, ~id, ~uri) => {
  WorkspaceData.{
    folders: [Folder.{uri, name, index: 0}],
    name,
    id,
    configuration: None,
    isUntitled: false,
  };
};

let workspace1 =
  toWorkspaceData(~name="Workspace1", ~id="workspace1", ~uri=workspace1Uri);
let workspace2 =
  toWorkspaceData(~name="Workspace2", ~id="workspace2", ~uri=workspace2Uri);

module TestWorkspaceEvent = {
  type t = {
    eventType: string,
    workspacePath: option(string),
    added: int,
    removed: int,
  };

  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          eventType: field.required("type", string),
          workspacePath: field.optional("workspacePath", string),
          added: field.required("added", int),
          removed: field.required("removed", int),
        }
      )
    );
  };
};

let waitForWorkspaceEvent = (~name, f, context) => {
  context
  |> Test.waitForMessage(
       ~name,
       fun
       | Msg.MessageService(ShowMessage({message, _})) => {
           message
           |> Yojson.Safe.from_string
           |> Json.Decode.decode_value(TestWorkspaceEvent.decode)
           |> Result.map(f)
           |> Result.value(~default=false);
         }
       | _ => false,
     );
};

describe("WorkspaceTest", ({test, _}) => {
  test("change workspaces", _ => {
    Test.startWithExtensions(["oni-workspace"])
    |> Test.waitForExtensionActivation("oni-workspace")
    |> Test.withClient(
         Request.Commands.executeContributedCommand(
           ~arguments=[],
           ~command="workspace.showActive",
         ),
       )
    |> waitForWorkspaceEvent(~name="Workspace showActive", evt => {
         evt.eventType == "workspace.show" && evt.workspacePath != None
       })
    |> Test.withClient(
         Request.Workspace.acceptWorkspaceData(~workspace=Some(workspace1)),
       )
    |> waitForWorkspaceEvent(~name="Workspace: changed to workspace 1", evt => {
         evt.eventType == "workspace.changed"
         && evt.workspacePath == Some(expectedPath1)
       })
    |> Test.withClient(
         Request.Workspace.acceptWorkspaceData(~workspace=Some(workspace2)),
       )
    |> waitForWorkspaceEvent(~name="Workspace: changed to workspace 2", evt => {
         evt.eventType == "workspace.changed"
         && evt.workspacePath == Some(expectedPath2)
       })
    |> Test.terminate
    |> Test.waitForProcessClosed
  })
});
