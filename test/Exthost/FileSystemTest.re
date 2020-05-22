
open Oni_Core;
open TestFramework;

open Exthost;

describe("FileSystem", ({describe, _}) => {
describe("proxy (exthost -> main)", ({test, _}) => {
  test("oni-proxy-filesystem extension activates", _ => {
    Test.startWithExtensions(["oni-proxy-filesystem"])
    |> Test.waitForExtensionActivation("oni-proxy-filesystem")
    |> Test.terminate
    |> Test.waitForProcessClosed
  });

  describe("$stat", ({test, _}) => {

    let statSuccess = Files.StatResult.{
      fileType: Files.FileType.Directory,
      mtime: 1,
      ctime: 2,
      size: 3,
    };
    
    let successHandler = fun
    | Msg.FileSystem(Stat({uri})) =>  {
      prerr_endline ("Got request!");
      let json =statSuccess
      |> Json.Encode.encode_value(Files.StatResult.encode);
      Lwt.return(Reply.okJson(json));
      }
    | _ => Lwt.return(Reply.okEmpty);

    let failHandler = fun
    | Msg.FileSystem(Stat(_)) =>
      Lwt.fail_with("Error!")
    | _ => Lwt.return(Reply.okEmpty);

      
    test("success case", _ => {
      Test.startWithExtensions(~handler=successHandler,
      ["oni-proxy-filesystem"])
      |> Test.waitForExtensionActivation("oni-proxy-filesystem")
      |> Test.withClient(
           Exthost.Request.Commands.executeContributedCommand(
             ~arguments=[`String("test-file")],
             ~command="fs.stat",
           ),
         )
    |> Test.waitForMessage(
         ~name="UnregisterSCMResourceGroup",
         fun
         | Msg.SCM(UnregisterSCMResourceGroup(_)) => true
         | _ => false,
       )
      |> Test.terminate
      |> Test.waitForProcessClosed
  
    });
    test("failure case", _ => {
      Test.startWithExtensions(~handler=failHandler,
      ["oni-proxy-filesystem"])
      |> Test.waitForExtensionActivation("oni-proxy-filesystem")
      |> Test.withClient(
           Exthost.Request.Commands.executeContributedCommand(
             ~arguments=[`String("test-file")],
             ~command="fs.stat",
           ),
         )
    |> Test.waitForMessage(
         ~name="UnregisterSCMResourceGroup",
         fun
         | Msg.SCM(UnregisterSCMResourceGroup(_)) => true
         | _ => false,
       )
      |> Test.terminate
      |> Test.waitForProcessClosed
  
    });

  });
});
});
