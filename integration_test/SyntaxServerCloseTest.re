open Oni_IntegrationTestLib;

SyntaxServerTest.run(
  ~name="SyntaxServerCloseTest",
  ({syntaxClient, hasExited, wait, isConnected}) => {
  wait(~name="Connected", isConnected);

  Oni_Syntax_Client.close(syntaxClient);

  // On a 'successful' close (no error), exit code should be 0
  wait(~name="Closed", () => hasExited() == Some(0));
});
