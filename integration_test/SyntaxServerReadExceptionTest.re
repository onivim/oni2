open Oni_IntegrationTestLib;

SyntaxServerTest.run(
  ~name="SyntaxServerReadExceptionTest", ({syntaxClient, hasExited, wait, _}) => {
  Oni_Syntax_Client.simulateReadException(syntaxClient);

  // Syntax server should exit with code 2
  wait(~name="Closed", () => hasExited() == Some(2));
});
