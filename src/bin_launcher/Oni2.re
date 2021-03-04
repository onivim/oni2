/*
 * Oni2.re
 *
 * This is the launcher for the editor.
 */

let stayAttached = ref(false);

let passthrough = Arg.Unit(() => ());
let passthroughFloat = Arg.Float(_ => ());
let passthroughString = Arg.String(_ => ());

let passthroughAndStayAttached = Arg.Set(stayAttached);
let passthroughFloatAndStayAttached = Arg.Float(_ => stayAttached := true);
let passthroughStringAndStayAttached = Arg.String(_ => stayAttached := true);

let spec =
  Arg.align([
    ("-c", passthroughString, "Execute an :ex command after loading files"),
    (
      "-f",
      Arg.Set(stayAttached),
      " Stay attached to the foreground terminal.",
    ),
    ("-v", passthroughAndStayAttached, " Print version information."),
    (
      "-f",
      Arg.Set(stayAttached),
      " Stay attached to the foreground terminal.",
    ),
    (
      "--nofork",
      Arg.Set(stayAttached),
      " Stay attached to the foreground terminal.",
    ),
    ("--debug", passthrough, " Enable debug logging."),
    (
      "--debug-exthost",
      passthrough,
      " Pipe exthost output to stdout/stderr.",
    ),
    ("--trace", passthrough, " Enable trace logging."),
    ("--quiet", passthrough, " Print only error log messages."),
    ("--silent", passthrough, " Do not print any logging."),
    ("--log-file", passthroughString, " Specify a file for the output logs."),
    ("--log-filter", passthroughString, " Filter log output."),
    (
      "--no-log-colors",
      passthrough,
      " Turn off colors and rich formatting in logs.",
    ),
    ("--checkhealth", passthrough, " Check the health of the Oni2 editor."),
    (
      "--disable-syntax-highlighting",
      passthrough,
      " Turn off syntax highlighting.",
    ),
    ("--disable-extensions", passthrough, " Turn off extension loading."),
    (
      "--disable-configuration",
      passthrough,
      " Do not load user configuration (use default configuration).",
    ),
    (
      "--gpu-acceleration",
      passthroughString,
      " Override default renderer strategy - one of: "
      ++ {|

      - auto: automatically choose between software / hardware rendering (default)
      - hardware: force hardware renderer
      - software: force software renderer
      |},
    ),
    (
      "--install-extension",
      passthroughStringAndStayAttached,
      " Install extension by specifying a path to the .vsix file",
    ),
    (
      "--query-extension",
      passthroughStringAndStayAttached,
      " Query extension info by specifying an extension id.",
    ),
    (
      "--uninstall-extension",
      passthroughStringAndStayAttached,
      " Uninstall extension by specifying an extension id.",
    ),
    (
      "--extensions-dir",
      passthroughString,
      " The folder to store/load VSCode extensions.",
    ),
    (
      "--list-extensions",
      passthroughAndStayAttached,
      " List the currently installed extensions.",
    ),
    (
      "--force-device-scale-factor",
      passthroughFloat,
      " Force the DPI scaling for the editor.",
    ),
    (
      "--working-directory",
      passthrough,
      " Set the current working for Oni2.",
    ),
    ("--version", passthroughAndStayAttached, " Print version information."),
    ("-v", passthroughAndStayAttached, " Print version information."),
  ]);

let usage = {|
Onivim 2

Usage:

  oni2 [options][paths...]

Options:
|};

let anonArg = _ => ();

/* NOTE: On MacOS, when launching the app through GateKeeper,
 * there is a legacy parameter parsed in: -psn_X_XXXXXX
 * Apparently this is a legacy ProcessSerialNumber - more info here:
 * http://mirror.informatimago.com/next/developer.apple.com/documentation/Carbon/Reference/Process_Manager/prmref_main/data_type_5.html#//apple_ref/doc/uid/TP30000208/C001951
 * https://stackoverflow.com/questions/10242115/os-x-strange-psn-command-line-parameter-when-launched-from-finder
 *
 * We fail and show an error message if we get an unrecognized parameter, so we need to filter this out, otherwise we get a crash on first launch:
 * https://github.com/onivim/oni2/issues/552
 */
let filterPsnArgument = args => {
  let psnRegex = Str.regexp("^-psn.*");
  let f = s => {
    !Str.string_match(psnRegex, s, 0);
  };

  args |> Array.to_list |> List.filter(f) |> Array.of_list;
};

let args = Sys.argv |> filterPsnArgument;

let () = {
  switch (Arg.parse_argv(args, spec, anonArg, usage)) {
  | exception (Arg.Bad(err)) =>
    prerr_endline(err);
    exit(1);
  | exception (Arg.Help(msg)) =>
    print_endline(msg);
    exit(0);
  | _ => ()
  };
};

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
    // For Windows, we need to use Sys.executable_name - Sys.argv is `./` when running
    // from %PATH% - see onivim/oni#872
    | Windows => Filename.dirname(Sys.executable_name)
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
  let cmdToRun = executingDirectory ++ executable;
  // The first argument is the executable, so we need to update that to point to 'Oni2_editor'
  args[0] = cmdToRun;
  Unix.create_process(cmdToRun, args, stdio, stdout, stderr);
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
