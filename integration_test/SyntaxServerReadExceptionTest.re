open Oni_IntegrationTestLib;

SyntaxServerTest.run(
  ~name="SyntaxServerReadExceptionTest",
  ({syntaxClient, hasExited, wait, isConnected}) => {
  wait(~name="Connected", isConnected);
  Oni_Syntax_Client.Testing.simulateReadException(syntaxClient);

  // Syntax server should exit with code 2
  wait(~name="Closed", () => hasExited() == Some(2));
});
