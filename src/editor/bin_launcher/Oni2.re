/*
 * Oni2.re
 *
 * This is the launcher for the editor.
 */

let stayAttached = ref(false);

let attach = () => {
  stayAttached := true;
};

let version = () => {
  print_endline("Onivim 2 0.0.0");
};

let spec = [
  ("-attach", Arg.Unit(attach), ""),
  ("-version", Arg.Unit(version), ""),
];

let anonArg = _ => ();

let () = Arg.parse(spec, anonArg, "Usage: ");

let startProcess = (stdio, stdout, stderr) => {
  let executingDirectory = Filename.dirname(Sys.argv[0]);
  print_endline("Executing directory: " ++ executingDirectory);
  Unix.create_process(
    executingDirectory ++ "/" ++ "Oni2_editor.exe",
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

    Unix.set_close_on_exec(pstdin);
    Unix.set_close_on_exec(stdin);
    Unix.set_close_on_exec(pstdout);
    Unix.set_close_on_exec(stdout);
    Unix.set_close_on_exec(pstderr);
    Unix.set_close_on_exec(stderr);

    print_endline("Starting process....");
    let _ = startProcess(pstdin, pstdout, pstderr);
    print_endline("Started!");

    Unix.close(pstdin);
    Unix.close(pstdout);
    Unix.close(pstderr);
  };

launch();
