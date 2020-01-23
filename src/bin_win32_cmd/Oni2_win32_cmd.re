/*
 * Oni2.re
 *
 * This is the launcher for the editor.
 */

/* NOTE: This is duplicated from Revery's core utility
 * This is the only method we use from Revery - this prevents us
 * from needing to bring in the full set of Revery's dependencies
 * just for this one method.
 */
let executingDirectory = {
  let dir =
    // For Windows, we need to use Sys.executable_name - Sys.argv is `./` when running
    // from %PATH% - see onivim/oni#872
    Filename.dirname(Sys.executable_name)

  /* Check if there is a trailing slash. If not, we need to add one. */
  let len = String.length(dir);
  switch (dir.[len - 1]) {
  | '/' => dir
  | '\\' => dir
  | _ => dir ++ Filename.dir_sep
  };
};

let executable = "Oni2.exe";

let startProcess = (stdio, stdout, stderr) => {
  let cmdToRun = executingDirectory ++ executable;
  let args = [
    // The first argument is the executable, so we need to update that to point to 'Oni2.exe'
    cmdToRun,
    // nofork - prepare GUI process to have stdout/stderr/etc
    "-f",
    ...Array.to_list(Sys.argv) // pass existing args
  ]
  |> Array.of_list;
  Unix.create_process(cmdToRun, args, stdio, stdout, stderr);
};

let launch = () => {
    let pid = startProcess(Unix.stdin, Unix.stdout, Unix.stderr);
    let (_, status) = Unix.waitpid([], pid);
    let exitCode =
      switch (status) {
      | WEXITED(v) => v
      | _ => 1
      };
    exit(exitCode);
};

launch();
