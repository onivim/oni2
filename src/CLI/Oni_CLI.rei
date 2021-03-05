open Oni_Core;

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

let default: t;

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

let parse: (~getenv: string => option(string), array(string)) => (t, eff);
