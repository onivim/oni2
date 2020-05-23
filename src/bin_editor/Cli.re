/**
 * Cli.re
 *
 * Module for handling command-line arguments for Oni2
 */
open Kernel;
open Rench;

module CoreLog = Log;
module Log = (val Log.withNamespace("Oni2.Core.Cli"));

type t = {
  folder: option(string),
  filesToOpen: list(string),
  forceScaleFactor: option(float),
  syntaxHighlightService: bool,
  overriddenExtensionsDir: option(string),
  shouldClose: bool,
  shouldLoadExtensions: bool,
  shouldSyntaxHighlight: bool,
  shouldLoadConfiguration: bool,
};

let noop = () => ();

let setWorkingDirectory = s => {
  Log.debug("--working-directory - chdir: " ++ s);
  Sys.chdir(s);
};

let setRef: (ref(option('a)), 'a) => unit =
  (someRef, v) => someRef := Some(v);

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

let parse =
    (
      ~checkHealth,
      ~listExtensions,
      ~installExtension,
      ~uninstallExtension,
      ~printVersion,
    ) => {
  let sysArgs = Sys.argv |> filterPsnArgument;

  let args: ref(list(string)) = ref([]);

  let scaleFactor = ref(None);
  let syntaxHighlightService = ref(false);
  let extensionsDir = ref(None);
  let shouldClose = ref(false);

  let shouldLoadExtensions = ref(true);
  let shouldLoadConfiguration = ref(true);
  let shouldSyntaxHighlight = ref(true);

  let needsConsole = ref(false);

  let queuedJob = ref(None);
  let runAndExitUnit = f =>
    Arg.Unit(
      () => {
        needsConsole := true;
        queuedJob := Some(cli => {f(cli) |> exit});
      },
    );

  let runAndExitString = f =>
    Arg.String(
      s => {
        needsConsole := true;
        queuedJob := Some(cli => {f(s, cli) |> exit});
      },
    );

  let disableExtensionLoading = () => shouldLoadExtensions := false;
  let disableLoadConfiguration = () => shouldLoadConfiguration := false;
  let disableSyntaxHighlight = () => shouldSyntaxHighlight := false;

  Arg.parse_argv(
    sysArgs,
    [
      ("-f", Unit(Timber.App.enable), ""),
      ("--nofork", Unit(Timber.App.enable), ""),
      ("--debug", Unit(CoreLog.enableDebug), ""),
      ("--trace", Unit(CoreLog.enableTrace), ""),
      ("--quiet", Unit(CoreLog.enableQuiet), ""),
      ("--version", printVersion |> runAndExitUnit, ""),
      ("--no-log-colors", Unit(Timber.App.disableColors), ""),
      ("--disable-extensions", Unit(disableExtensionLoading), ""),
      ("--disable-configuration", Unit(disableLoadConfiguration), ""),
      ("--disable-syntax-highlighting", Unit(disableSyntaxHighlight), ""),
      ("--log-file", String(Timber.App.setLogFile), ""),
      ("--log-filter", String(Timber.App.setNamespaceFilter), ""),
      ("--checkhealth", checkHealth |> runAndExitUnit, ""),
      ("--list-extensions", listExtensions |> runAndExitUnit, ""),
      ("--install-extension", installExtension |> runAndExitString, ""),
      ("--uninstall-extension", uninstallExtension |> runAndExitString, ""),
      ("--working-directory", String(setWorkingDirectory), ""),
      (
        "--force-device-scale-factor",
        Float(f => scaleFactor := Some(f)),
        "",
      ),
      (
        "--syntax-highlight-service",
        Unit(() => syntaxHighlightService := true),
        "",
      ),
      ("--extensions-dir", String(setRef(extensionsDir)), ""),
      ("--force-device-scale-factor", Float(setRef(scaleFactor)), ""),
    ],
    arg => args := [arg, ...args^],
    "",
  );

  if (Timber.App.isEnabled() || needsConsole^) {
    /* On Windows, we need to create a console instance if possible */
    Revery.App.initConsole();
  };

  let paths = args^ |> List.rev;
  let workingDirectory = Environment.getWorkingDirectory();

  let stripTrailingPathCharacter = s => {
    let len = String.length(s);
    if (len > 1 && (s.[len - 1] == '/' || s.[len - 1] == '\\')) {
      String.sub(s, 0, len - 1);
    } else {
      s;
    };
  };

  let isAbsolutePathWithTilde = s =>
    switch (s.[0]) {
    | '~' => !Sys.win32
    | _ => false
    | exception (Invalid_argument(_)) => false
    };

  let resolvePath = p => {
    let p =
      if (Path.isAbsolute(p) || isAbsolutePathWithTilde(p)) {
        p;
      } else {
        Path.join(workingDirectory, p);
      };

    p |> Path.normalize |> stripTrailingPathCharacter;
  };

  let absolutePaths = List.map(resolvePath, paths);

  let isDirectory = p => {
    switch (Sys.is_directory(p)) {
    | v => v
    | exception (Sys_error(_)) => false
    };
  };

  let directories = List.filter(isDirectory, absolutePaths);
  let filesToOpen = List.filter(p => !isDirectory(p), absolutePaths);

  let folder =
    switch (directories) {
    | [first, ..._] => Some(first)
    | [] =>
      switch (filesToOpen) {
      | [first, ..._] => Some(Rench.Path.dirname(first))
      | [] => None
      }
    };

  let cli = {
    folder,
    filesToOpen,
    forceScaleFactor: scaleFactor^,
    syntaxHighlightService: syntaxHighlightService^,
    overriddenExtensionsDir: extensionsDir^,
    shouldClose: shouldClose^,
    shouldSyntaxHighlight: shouldSyntaxHighlight^,
    shouldLoadExtensions: shouldLoadExtensions^,
    shouldLoadConfiguration: shouldLoadConfiguration^,
  };

  switch (queuedJob^) {
  | None => ()
  | Some(job) => job(cli)
  };

  cli;
};
