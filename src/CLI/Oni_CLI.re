/**
 * Cli.re
 *
 * Module for handling command-line arguments for Oni2
 */
open Oni_Core;
open Kernel;
open Rench;

module CoreLog = Log;
module Log = (val Log.withNamespace("Oni2.Core.Cli"));

type t = {
  gpuAcceleration: [ | `Auto | `ForceSoftware | `ForceHardware],
  folder: option(string),
  filesToOpen: list(string),
  forceScaleFactor: option(float),
  overriddenExtensionsDir: option(FpExp.t(FpExp.absolute)),
  shouldLoadExtensions: bool,
  shouldLoadConfiguration: bool,
  shouldSyntaxHighlight: bool,
  attachToForeground: bool,
  logExthost: bool,
  logLevel: option(Timber.Level.t),
  logFile: option(string),
  logFilter: option(string),
  logColorsEnabled: option(bool),
  needsConsole: bool,
  vimExCommands: list(string),
};

type eff =
  | PrintVersion
  | CheckHealth
  | ListExtensions
  | InstallExtension(string)
  | QueryExtension(string)
  | UninstallExtension(string)
  | StartSyntaxServer({
      parentPid: string,
      namedPipe: string,
    })
  | Run;

let parseSyntaxArgs = str => {
  str
  |> String.split_on_char(':')
  |> (
    fun
    | [pid, namedPipe] => (pid, namedPipe)
    | _ => failwith("Unexpected argument format for syntax server: " ++ str)
  );
};

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

let parse = (~getenv: string => option(string), args) => {
  let sysArgs = args |> filterPsnArgument;

  let additionalArgs: ref(list(string)) = ref([]);

  let scaleFactor = ref(None);
  let extensionsDir = ref(None);
  let eff = ref(Run);

  let shouldLoadExtensions = ref(true);
  let shouldLoadConfiguration = ref(true);
  let shouldSyntaxHighlight = ref(true);

  let attachToForeground = ref(false);
  let logLevel = ref(None);
  let isSilent = ref(false);
  let logExthost = ref(false);
  let logFile = ref(None);
  let logFilter = ref(None);
  let logColorsEnabled = ref(None);
  let gpuAcceleration = ref(`Auto);
  let vimExCommands = ref([]);

  let setGpuAcceleration =
    fun
    | "software" => gpuAcceleration := `ForceSoftware
    | "hardware" => gpuAcceleration := `ForceHardware
    | _unknown => ();

  let setEffect = effect => {
    Arg.Unit(() => {eff := effect});
  };

  let setStringEffect = (f: string => eff) => {
    Arg.String(s => {eff := f(s)});
  };

  let disableExtensionLoading = () => shouldLoadExtensions := false;
  let disableLoadConfiguration = () => shouldLoadConfiguration := false;
  let disableSyntaxHighlight = () => shouldSyntaxHighlight := false;

  let setAttached = () => {
    attachToForeground := true;
    // Set log level if it hasn't already been set
    switch (logLevel^) {
    | None => logLevel := Some(Timber.Level.info)
    | Some(_) => ()
    };
  };

  getenv("ONI2_LOG_FILE") |> Option.iter(v => logFile := Some(v));

  getenv("ONI2_DEBUG")
  |> Option.iter(_ => logLevel := Some(Timber.Level.debug));

  getenv("ONI2_LOG_FILTER") |> Option.iter(v => logFilter := Some(v));

  Arg.parse_argv(
    ~current=ref(0),
    sysArgs,
    [
      ("-c", String(str => vimExCommands := [str, ...vimExCommands^]), ""),
      ("-f", Unit(setAttached), ""),
      ("-v", setEffect(PrintVersion), ""),
      ("--nofork", Unit(setAttached), ""),
      ("--debug", Unit(() => logLevel := Some(Timber.Level.debug)), ""),
      ("--debug-exthost", Unit(() => logExthost := true), ""),
      ("--trace", Unit(() => logLevel := Some(Timber.Level.trace)), ""),
      ("--quiet", Unit(() => logLevel := Some(Timber.Level.warn)), ""),
      (
        "--silent",
        Unit(
          () => {
            logLevel := None;
            isSilent := true;
          },
        ),
        "",
      ),
      ("--version", setEffect(PrintVersion), ""),
      ("--no-log-colors", Unit(() => logColorsEnabled := Some(false)), ""),
      ("--disable-extensions", Unit(disableExtensionLoading), ""),
      ("--disable-configuration", Unit(disableLoadConfiguration), ""),
      ("--disable-syntax-highlighting", Unit(disableSyntaxHighlight), ""),
      ("--gpu-acceleration", String(setGpuAcceleration), ""),
      ("--log-file", String(str => logFile := Some(str)), ""),
      ("--log-filter", String(str => logFilter := Some(str)), ""),
      ("--checkhealth", setEffect(CheckHealth), ""),
      ("--list-extensions", setEffect(ListExtensions), ""),
      ("--install-extension", setStringEffect(s => InstallExtension(s)), ""),
      ("--query-extension", setStringEffect(s => QueryExtension(s)), ""),
      (
        "--uninstall-extension",
        setStringEffect(s => UninstallExtension(s)),
        "",
      ),
      ("--working-directory", String(setWorkingDirectory), ""),
      (
        "--force-device-scale-factor",
        Float(f => scaleFactor := Some(f)),
        "",
      ),
      (
        "--syntax-highlight-service",
        setStringEffect(syntaxArgs =>
          syntaxArgs
          |> parseSyntaxArgs
          |> (
            ((pid, namedPipe)) =>
              StartSyntaxServer({parentPid: pid, namedPipe})
          )
        ),
        "",
      ),
      ("--extensions-dir", String(setRef(extensionsDir)), ""),
      ("--force-device-scale-factor", Float(setRef(scaleFactor)), ""),
    ],
    arg => additionalArgs := [arg, ...additionalArgs^],
    "",
  );

  let shouldAlwaysAllocateConsole =
    switch (eff^) {
    | Run => false
    | StartSyntaxServer(_) => false
    | _ => true
    };

  let needsConsole =
    (isSilent^ || Option.is_some(logLevel^))
    && attachToForeground^
    || shouldAlwaysAllocateConsole;

  let paths = additionalArgs^ |> List.rev;

  let isAnonymousExCommand = str => String.length(str) > 0 && str.[0] == '+';
  let anonymousExCommands =
    paths
    |> List.filter(isAnonymousExCommand)
    |> List.map(str => String.sub(str, 1, String.length(str) - 1));

  let paths = paths |> List.filter(str => !isAnonymousExCommand(str));

  let workingDirectory = Environment.getWorkingDirectory();

  let stripTrailingPathCharacter = s => {
    let len = String.length(s);
    if (len > 1 && (s.[len - 1] == '/' || s.[len - 1] == '\\')) {
      String.sub(s, 0, len - 1);
    } else {
      s;
    };
  };

  let isDrive = s => {
    // Check for a path of the form [a-z]:
    Sys.win32 && String.length(s) == 3 && s.[1] == ':';
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
        {
          Path.join(workingDirectory, p);
        }
        |> Path.normalize;
      };

    if (!isDrive(p)) {
      stripTrailingPathCharacter(p);
    } else {
      p;
    };
  };

  let absolutePaths = List.map(resolvePath, paths);

  let isDirectory = p => {
    switch (isDrive(p) || Sys.is_directory(p)) {
    | v => v
    | exception (Sys_error(_)) => false
    };
  };

  let directories = List.filter(isDirectory, absolutePaths);
  let filesToOpen = List.filter(p => !isDirectory(p), absolutePaths);

  let folder =
    switch (directories) {
    | [first, ..._] => Some(first)
    | [] => None
    };

  let cli = {
    folder,
    filesToOpen,
    forceScaleFactor: scaleFactor^,
    gpuAcceleration: gpuAcceleration^,
    overriddenExtensionsDir:
      extensionsDir^
      |> Utility.OptionEx.flatMap(FpExp.absoluteCurrentPlatform),
    shouldLoadExtensions: shouldLoadExtensions^,
    shouldLoadConfiguration: shouldLoadConfiguration^,
    shouldSyntaxHighlight: shouldSyntaxHighlight^,
    attachToForeground: attachToForeground^,
    logLevel: logLevel^,
    logExthost: logExthost^,
    logFile: logFile^,
    logFilter: logFilter^,
    logColorsEnabled: logColorsEnabled^,
    needsConsole,
    vimExCommands: (vimExCommands^ |> List.rev) @ anonymousExCommands,
  };

  (cli, eff^);
};

let default = fst(parse(~getenv=_ => None, [||]));
