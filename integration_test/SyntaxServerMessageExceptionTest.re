open Oni_Core;
open Oni_Core.Utility;
open Oni_Extensions;

open Oni_IntegrationTestLib;

SyntaxServerTest.run(
  ~name="SyntaxServerMessageExceptionTest",
  ({syntaxClient, isConnected, hasExited, wait}) => {
  // Simulate an error parsing a message
  Oni_Syntax_Client.simulateMessageException(syntaxClient);

  // Syntax server should exit with code 2
  wait(~name="Closed", () => hasExited() == Some(2));
});
