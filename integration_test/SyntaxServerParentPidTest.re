open Oni_IntegrationTestLib;

SyntaxServerTest.run(
  ~name="SyntaxServerParentPidTest", ~parentPid="-1", ({hasExited, wait, _}) => {
  // This should close automatically, since there is no parentPid
  // present...
  wait(~name="Closed", () =>
    hasExited() == Some(0)
  )
});
