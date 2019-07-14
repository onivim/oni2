/*
 * Oni2.re
 *
 * This is the launcher for the editor.
 */

let stayAttached = ref(false);

let version = () => {
  print_endline("Onivim 2 0.0.0");
};

let passthrough = Arg.Unit(() => ());

let spec = [
  ("-f", Arg.Set(stayAttached), ""),
  ("--nofork", Arg.Set(stayAttached), ""),
  ("--checkhealth", passthrough, ""),
  ("-version", Arg.Unit(version), ""),
];

let anonArg = _ => ();

let () = Arg.parse(spec, anonArg, "Usage: ");

type platform =
  | Windows
  | Mac
  | Linux
  | Unknown;

let os =
  switch (Sys.os_type) {
  | "Win32" => Windows
  | _ =>
    let ic = Unix.open_process_in("uname");
    let uname = input_line(ic);
    let _ = close_in(ic);
    switch (uname) {
    | "Darwin" => Mac
    | "Linux" => Linux
    | _ => Unknown
    };
  };

/* NOTE: This is duplicated from Revery's core utility
 * This is the only method we use from Revery - this prevents us
 * from needing to bring in the full set of Revery's dependencies
 * just for this one method.
 */
let executingDirectory = {
  let dir =
    switch (os) {
    /* The default strategy of preferring Sys.executable_name seems to only work
     * reliably on Mac.  On Linux, it will return the symlink source instead of
     * the symlink destination - this causes problems when trying to load assets
     * relative to the binary location when symlinked.
     */
    | Mac =>
      switch (
        String.rindex_opt(Sys.executable_name, '/'),
        String.rindex_opt(Sys.executable_name, '\\'),
      ) {
      | (Some(v1), Some(v2)) =>
        String.sub(Sys.executable_name, 0, max(v1, v2))
      | (None, Some(v)) => String.sub(Sys.executable_name, 0, v)
      | (Some(v), None) => String.sub(Sys.executable_name, 0, v)
      | _ => Sys.executable_name
      }
    | _ => Filename.dirname(Sys.argv[0]) ++ Filename.dir_sep
    };

  /* Check if there is a trailing slash. If not, we need to add one. */

  let len = String.length(dir);
  switch (dir.[len - 1]) {
  | '/' => dir
  | '\\' => dir
  | _ => dir ++ Filename.dir_sep
  };
};

let executable = Sys.win32 ? "Oni2_editor.exe" : "Oni2_editor";

let startProcess = (stdio, stdout, stderr) => {
  Unix.create_process(
    executingDirectory ++ executable,
    Sys.argv,
    stdio,
    stdout,
    stderr,
  );
};

let launch = () =>
  if (stayAttached^) {
    let pid = startProcess(Unix.stdin, Unix.stdout, Unix.stderr);
    let (_, status) = Unix.waitpid([], pid);
    let exitCode =
      switch (status) {
      | WEXITED(v) => v
      | _ => 1
      };
    exit(exitCode);
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
