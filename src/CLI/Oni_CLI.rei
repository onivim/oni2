type t = {
  folder: option(string),
  filesToOpen: list(string),
  forceScaleFactor: option(float),
  overriddenExtensionsDir: option(string),
  shouldClose: bool,
  shouldLoadExtensions: bool,
  shouldLoadConfiguration: bool,
  shouldSyntaxHighlight: bool,
};

type eff =
  | PrintVersion
  | CheckHealth
  | ListExtensions
  | InstallExtension(string)
  | UninstallExtension(string)
  | StartSyntaxServer({
      parentPid: string,
      namedPipe: string,
    })
  | Run;

let parse: array(string) => (t, eff);
