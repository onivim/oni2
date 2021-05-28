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

describe("HttpRequestTest", ({test, _}) => {
  test("Execute a (url, cb) request in extension host", _ => {
    Test.startWithExtensions(["oni-http-request"])
    |> Test.waitForExtensionActivation("oni-http-request")
    |> Test.withClient(
         Request.Commands.executeContributedCommand(
           ~arguments=[],
           ~command="http.request2",
         ),
       )
    |> waitForMessage(
         ~name="wait for request result (2-param)", (severity, message) =>
         switch (severity) {
         | Info => true
         | Error =>
           prerr_endline("Got error: " ++ message);
           false;
         | _ => assert(false)
         }
       )
    |> Test.terminate
    |> Test.waitForProcessClosed
  });

  // Regression test for #2981 - ensure 3-param requests work correctly
  test("#2981: Execute a (url, options, cb) request in extension host", _ => {
    Test.startWithExtensions(["oni-http-request"])
    |> Test.waitForExtensionActivation("oni-http-request")
    |> Test.withClient(
         Request.Commands.executeContributedCommand(
           ~arguments=[],
           ~command="http.request3",
         ),
       )
    |> waitForMessage(
         ~name="wait for request result (3-param)", (severity, message) =>
         switch (severity) {
         | Info => true
         | Error =>
           prerr_endline("Got error: " ++ message);
           false;
         | _ => assert(false)
         }
       )
    |> Test.terminate
    |> Test.waitForProcessClosed
  });
});
