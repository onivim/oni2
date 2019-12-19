/*
 * ExtensionManifest.re
 *
 * Module to describing metadata about an extension
 */
module Path = Rench.Path;
module Option = Oni_Core.Utility.Option;

module ExtensionKind = {
  [@deriving (show, yojson({strict: false, exn: true}))]
  type t =
    | [@name "ui"] Ui
    | [@name "workspace"] Workspace;
};

module Engine = {
  [@deriving (show, yojson({strict: false, exn: true}))]
  type t = {vscode: string};
};

[@deriving (show, yojson({strict: false, exn: true}))]
type t = {
  name: string,
  version: string,
  displayName: [@default None] option(string),
  description: [@default None] option(string),
  main: [@default None] option(string),
  icon: [@default None] option(string),
  categories: [@default []] list(string),
  keywords: [@default []] list(string),
  engines: Engine.t,
  activationEvents: [@default []] list(string),
  extensionDependencies: [@default []] list(string),
  extensionPack: [@default []] list(string),
  extensionKind: [@default Ui] ExtensionKind.t,
  contributes: ExtensionContributions.t,
};

let remapPaths = (rootPath: string, manifest: t) => {
  ...manifest,
  main: Option.map(mainPath => Path.join(rootPath, mainPath), manifest.main),
  icon: Option.map(iconPath => Path.join(rootPath, iconPath), manifest.icon),
  contributes:
    ExtensionContributions.remapPaths(rootPath, manifest.contributes),
};

let updateName = (nameSetter, manifest: t) => {
  ...manifest,
  name: nameSetter(manifest.name),
};
