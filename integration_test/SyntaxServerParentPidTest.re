open Oni_IntegrationTestLib;

// TODO: Is there a way we can pick a PID that _for sure_ is not being used?
// If there happens to be another process with this pid running, we'll fail...
// A better strategy might be to spin up a 'dummy 'process, use that PID -
// and then close it, and validate the syntax server exits.
let parentPid = "42";

SyntaxServerTest.run(
  ~name="SyntaxServerParentPidTest", ~parentPid, ({hasExited, wait, _}) => {
  // This should close automatically, since there is no parentPid
  // present...
  wait(~name="Closed", ~timeout=30.0, ()
    // TODO: This should be exit code 0,
    // but there is a bug on Windows preventing this - the 'wait_pid' behavior.
    // hasExited() == Some(0)
    => Option.is_some(hasExited()))
});
