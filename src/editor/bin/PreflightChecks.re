/*
 * PreflightChecks.re
 *
 * This establishes and verifies a set of invariants to ensure the environment
 * is set up correctly for the application to run.
 */

let checkHomeDirectoryOrThrow = () => {
  let home = Filesystem.unsafeFindHome();
  ();
};

let run = () => {
  checkHomeDirectoryorThrow();
};
