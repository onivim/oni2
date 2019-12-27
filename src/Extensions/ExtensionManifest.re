/*
 * ExtensionManifest.re
 *
 * Module to describing metadata about an extension
 */

open Oni_Core.Utility;

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
  displayName: [@default None] option(LocalizedToken.t),
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
  contributes:
    ExtensionContributions.remapPaths(rootPath, manifest.contributes),
};

let updateName = (nameSetter, manifest: t) => {
  ...manifest,
  name: nameSetter(manifest.name),
};

let localize = (loc: LocalizationDictionary.t, manifest: t) => {
  ...manifest,
  displayName:
    Option.map(LocalizedToken.localize(loc), manifest.displayName),
  contributes: ExtensionContributions.localize(loc, manifest.contributes),
};
