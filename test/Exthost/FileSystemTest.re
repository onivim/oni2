open Oni_Core;
open TestFramework;

open Exthost;

module TestResponse = {
  type t('a) = result('a, string);

  let decode = decoder => {
    Json.Decode.(
      {
        obj(({field, _}) => {
          let result = field.required("result", string);

          if (result == "success") {
            let value = field.required("payload", decoder);
            Ok(value);
          } else {
            let errMsg = field.required("error", decoder);
            Error(errMsg);
          };
        });
      }
    );
  };
};

let waitForFilesystemEvent = (~name, ~decoder, f, context) => {
  context
  |> Test.waitForMessage(
       ~name,
       fun
       | Msg.MessageService(ShowMessage({message, _})) => {
           message
           |> Yojson.Safe.from_string
           |> Json.Decode.decode_value(TestResponse.decode(decoder))
           |> Result.map(f)
           |> Result.value(~default=false);
         }
       | _ => false,
     );
};

describe("FileSystem", ({describe, _}) => {
  describe("proxy (exthost -> main)", ({test, _}) => {
    test("oni-proxy-filesystem extension activates", _ => {
      Test.startWithExtensions(["oni-proxy-filesystem"])
      |> Test.waitForExtensionActivation("oni-proxy-filesystem")
      |> Test.terminate
      |> Test.waitForProcessClosed
    });

    describe("$stat", ({test, _}) => {
      let statSuccess =
        Files.StatResult.{
          fileType: Files.FileType.Directory,
          mtime: 1,
          ctime: 2,
          size: 3,
        };

      let successHandler =
        fun
        | Msg.FileSystem(Stat(_)) => {
            let json =
              statSuccess |> Json.Encode.encode_value(Files.StatResult.encode);
            Lwt.return(Reply.okJson(json));
          }
        | _ => Lwt.return(Reply.okEmpty);

      let failHandler =
        fun
        | Msg.FileSystem(Stat(_)) => Lwt.fail_with("Error!")
        | _ => Lwt.return(Reply.okEmpty);

      test("success case", _ => {
        Test.startWithExtensions(
          ~handler=successHandler,
          ["oni-proxy-filesystem"],
        )
        |> Test.waitForExtensionActivation("oni-proxy-filesystem")
        |> Test.withClient(
             Exthost.Request.Commands.executeContributedCommand(
               ~arguments=[`String("test-file")],
               ~command="fs.stat",
             ),
           )
        |> waitForFilesystemEvent(
             ~name="success result",
             ~decoder=Json.Decode.value,
             fun
             | Ok(_) => true
             | Error(_) => false,
           )
        |> Test.terminate
        |> Test.waitForProcessClosed
      });
      test("failure case", _ => {
        Test.startWithExtensions(
          ~handler=failHandler,
          ["oni-proxy-filesystem"],
        )
        |> Test.waitForExtensionActivation("oni-proxy-filesystem")
        |> Test.withClient(
             Exthost.Request.Commands.executeContributedCommand(
               ~arguments=[`String("test-file")],
               ~command="fs.stat",
             ),
           )
        |> waitForFilesystemEvent(
             ~name="success result",
             ~decoder=Json.Decode.value,
             fun
             | Ok(_) => false
             | Error(_) => true,
           )
        |> Test.terminate
        |> Test.waitForProcessClosed
      });
    });
    describe("$writeFile", ({test, _}) => {
      test("gets buffer from request", ({expect, _}) => {
        Test.startWithExtensions(["oni-proxy-filesystem"])
        |> Test.waitForExtensionActivation("oni-proxy-filesystem")
        |> Test.withClient(
             Exthost.Request.Commands.executeContributedCommand(
               ~arguments=[`String("test-file")],
               ~command="fs.write",
             ),
           )
        |> Test.waitForMessage(
             ~name="$writeFile",
             fun
             | Msg.FileSystem(WriteFile({bytes, _})) => {
                 let len = bytes |> Bytes.length;
                 expect.equal(len, 1234);
                 true;
               }
             | _ => false,
           )
        |> Test.terminate
        |> Test.waitForProcessClosed
      })
    });
  });
  describe("$readFile", ({test, _}) => {
    let successHandler =
      fun
      | Msg.FileSystem(ReadFile(_)) => {
          Lwt.return(Reply.okBuffer(Bytes.of_string("Hello from Reason!")));
        }
      | _ => Lwt.return(Reply.okEmpty);

    test("success case", _ => {
      Test.startWithExtensions(
        ~handler=successHandler,
        ["oni-proxy-filesystem"],
      )
      |> Test.waitForExtensionActivation("oni-proxy-filesystem")
      |> Test.withClient(
           Exthost.Request.Commands.executeContributedCommand(
             ~arguments=[],
             ~command="fs.read",
           ),
         )
      |> waitForFilesystemEvent(
           ~name="success result",
           ~decoder=Json.Decode.string,
           fun
           | Ok(msg) => {
               String.equal(msg, "Hello from Reason!");
             }
           | Error(_) => false,
         )
      |> Test.terminate
      |> Test.waitForProcessClosed
    });
  });
});
