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

type eff =
  | PrintVersion
  | CheckHealth
  | ListExtensions
  | InstallExtension(string)
  | UninstallExtension(string)
  | Run;

let parse: array(string) => (t, eff);
