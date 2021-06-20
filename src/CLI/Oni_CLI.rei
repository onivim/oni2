open Oni_Core;

type position = {
  x: int,
  y: int,
};

type t = {
  gpuAcceleration: [ | `Auto | `ForceSoftware | `ForceHardware],
  folder: option(string),
  filesToOpen: list(string),
  forceScaleFactor: option(float),
  forceNewWindow: bool,
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
  proxyServer: Service_Net.Proxy.t,
  vimExCommands: list(string),
  windowPosition: option(position),
};

let default: t;

type eff =
  | PrintVersion
  | CheckHealth
  | DoRemoteCommand({
      pipe: string,
      filesToOpen: list(string),
      folder: option(string),
    })
  | ListDisplays
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
