/*
 * ExtensionManifest.re
 *
 * Module to describing metadata about an extension
 */
open Oni_Core_Utility;
module Path = Rench.Path;

module ExtensionKind = {
  [@deriving (show, yojson({strict: false, exn: true}))]
  type t =
    | [@name "ui"] Ui
    | [@name "workspace"] Workspace;
};

module Author = {
  [@deriving show]
  type t = option(string);

  let of_yojson_exn =
    fun
    | `String(v) => Some(v)
    | v =>
      Yojson.Safe.(
        {
          Util.member("name", v) |> Util.to_string_option;
        }
      );

  let of_yojson = json =>
    ResultEx.guard(() => of_yojson_exn(json))
    |> Result.map_error(_ => "Error parsing author");

  let to_yojson = _author => `Null;
};

module Engine = {
  [@deriving (show, yojson({strict: false, exn: true}))]
  type t = {vscode: string};
};

[@deriving (show, yojson({strict: false, exn: true}))]
type t = {
  name: string,
  version: string,
  author: [@default None] Author.t,
  displayName: [@default None] option(LocalizedToken.t),
  description: [@default None] option(string),
  publisher: [@default None] option(string),
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
  enableProposedApi: [@default false] bool,
};

let getDisplayName = (manifest: t) => {
  manifest.displayName
  |> Option.map(tok => LocalizedToken.to_string(tok))
  |> Option.value(~default=manifest.name);
};

let getAuthor = manifest => {
  manifest.author
  |> OptionEx.or_(manifest.publisher)
  |> Option.value(~default="Unknown Author");
};

let getVersion = manifest => manifest.version;

let getIcon = (manifest: t) => manifest.icon;

let remapPaths = (rootPath: string, manifest: t) => {
  ...manifest,
  main: Option.map(Path.join(rootPath), manifest.main),
  icon: Option.map(Path.join(rootPath), manifest.icon),
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
