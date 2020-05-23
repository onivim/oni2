open TestFramework;

open Exthost;

let waitForWriteText = (f, context) => {
  context
  |> Test.waitForMessage(
       ~name="Wait for Clipboard(WriteText(msg))",
       fun
       | Msg.Clipboard(WriteText(message)) => f(message)
       | _ => false,
     );
};

describe("ClipboardTest", ({test, _}) => {
  test("write text", _ => {
    Test.startWithExtensions(["oni-clipboard"])
    |> Test.waitForExtensionActivation("oni-clipboard")
    |> Test.withClient(
         Request.Commands.executeContributedCommand(
           ~arguments=[`String("hello clipboard")],
           ~command="clipboard.setText",
         ),
       )
    |> waitForWriteText(String.equal("hello clipboard"))
    |> Test.terminate
    |> Test.waitForProcessClosed
  });
  test("read text success", _ => {
    let handler =
      fun
      | Msg.Clipboard(ReadText) =>
        Lwt.return(Reply.okJson(`String("clipboard text")))
      | _ => Lwt.return(Reply.okEmpty);

    Test.startWithExtensions(~handler, ["oni-clipboard"])
    |> Test.waitForExtensionActivation("oni-clipboard")
    |> Test.withClient(
         Request.Commands.executeContributedCommand(
           ~arguments=[],
           ~command="clipboard.getText",
         ),
       )
    |> waitForWriteText(String.equal("success:clipboard text"))
    |> Test.terminate
    |> Test.waitForProcessClosed;
  });
  test("read text failure", _ => {
    let handler =
      fun
      | Msg.Clipboard(ReadText) =>
        Lwt.return(Reply.error("can't handle it"))
      | _ => Lwt.return(Reply.okEmpty);

    Test.startWithExtensions(~handler, ["oni-clipboard"])
    |> Test.waitForExtensionActivation("oni-clipboard")
    |> Test.withClient(
         Request.Commands.executeContributedCommand(
           ~arguments=[],
           ~command="clipboard.getText",
         ),
       )
    |> waitForWriteText(String.equal("failed"))
    |> Test.terminate
    |> Test.waitForProcessClosed;
  });
});
