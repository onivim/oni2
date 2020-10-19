type t = {
  gpuAcceleration: [ | `Auto | `ForceSoftware | `ForceHardware],
  folder: option(string),
  filesToOpen: list(string),
  forceScaleFactor: option(float),
  overriddenExtensionsDir: option(string),
//  shouldClose: bool,
  shouldLoadExtensions: bool,
  shouldLoadConfiguration: bool,
  shouldSyntaxHighlight: bool,
  attachToForeground: bool,
  logLevel: option(Timber.Level.t),
  logFile: option(string),
  logFilter: option(string),
  logColorsEnabled: option(bool),
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

let parse: array(string) => (t, eff);
