open TestFramework;

open Oni_Core;
open Exthost;

let waitForMessage = (~name, f, context) => {
  context
  |> Test.waitForMessage(
       ~name,
       fun
       | Msg.MessageService(ShowMessage({message, severity, _})) =>
         f(severity, message)
       | _ => false,
     );
};

describe("UriTest", ({test, _}) => {
  test("revive uri", _ => {
    Test.startWithExtensions(["oni-uri"])
    |> Test.waitForExtensionActivation("oni-uri")
    |> Test.withClient(
         Request.Commands.executeContributedCommand(
           ~arguments=[Uri.encode(Uri.fromPath("/"))],
           ~command="developer.oni.reviveUri",
         ),
       )
    |> waitForMessage(~name="revive uri success", (severity, _message) =>
         switch (severity) {
         | Info => true
         | Error => false
         | _ => assert(false)
         }
       )
    |> Test.terminate
    |> Test.waitForProcessClosed
  })
});
