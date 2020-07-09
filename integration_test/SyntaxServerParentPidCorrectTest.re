open Oni_IntegrationTestLib;

SyntaxServerTest.run(
  ~name="SyntaxServerParentPidCorrectTest", ({hasExited, wait, _}) => {
  Unix.sleepf(10.0);

  // Simply validate the syntax server has stayed alive...
  wait(~name="Not closed", ()
    // Validate we haven't closed after ten seconds
    => Option.is_none(hasExited()));
});
