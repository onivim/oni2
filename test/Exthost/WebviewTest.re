open TestFramework;

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

describe("WebviewTest", ({test, _}) => {
  test("revive uri", _ => {
    Test.startWithExtensions(["oni-webview"])
    |> Test.waitForExtensionActivation("oni-webview")
    |> Test.withClient(
         Request.Commands.executeContributedCommand(
           ~arguments=[],
           ~command="webview.test.asWebviewUri",
         ),
       )
    |> waitForMessage(~name="api call success", (severity, _message) =>
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
