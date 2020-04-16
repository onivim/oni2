open Oni_IntegrationTestLib;

SyntaxServerTest.run(
  ~name="SyntaxServerMessageExceptionTest",
  ({syntaxClient, hasExited, wait, isConnected}) => {
  wait(~name="Connected", isConnected);

  // Simulate an error parsing a message
  Oni_Syntax_Client.Testing.simulateMessageException(syntaxClient);

  // Syntax server should exit with code 2
  wait(~name="Closed", () => hasExited() == Some(2));
});
