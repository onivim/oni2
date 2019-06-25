/*
 * Oni2.re
 *
 * This is the launcher for the editor.
 */

let stayAttached = ref(false);

let version = () => {
  print_endline("Onivim 2 0.0.0");
};

let spec = [
  ("-f", Arg.Set(stayAttached), ""),
  ("--nofork", Arg.Set(stayAttached), ""),
  ("-version", Arg.Unit(version), ""),
];

let anonArg = _ => ();

let () = Arg.parse(spec, anonArg, "Usage: ");

let executable = Sys.win32 ? "Oni2_editor.exe" : "Oni2_editor";

let startProcess = (stdio, stdout, stderr) => {
  let executingDirectory = Filename.dirname(Sys.argv[0]);
  Unix.create_process(
    executingDirectory ++ "/" ++ executable,
    Sys.argv,
    stdio,
    stdout,
    stderr,
  );
};

let launch = () =>
  if (stayAttached^) {
    let pid = startProcess(Unix.stdin, Unix.stdout, Unix.stderr);
    let _ = Unix.waitpid([], pid);
    ();
  } else {
    let (pstdin, stdin) = Unix.pipe();
    let (stdout, pstdout) = Unix.pipe();
    let (stderr, pstderr) = Unix.pipe();

    let _ = startProcess(pstdin, pstdout, pstderr);

    Unix.set_close_on_exec(pstdin);
    Unix.set_close_on_exec(stdin);
    Unix.set_close_on_exec(pstdout);
    Unix.set_close_on_exec(stdout);
    Unix.set_close_on_exec(pstderr);
    Unix.set_close_on_exec(stderr);

    Unix.close(pstdin);
    Unix.close(pstdout);
    Unix.close(pstderr);
  };

launch();
