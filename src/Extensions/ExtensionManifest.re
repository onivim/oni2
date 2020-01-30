/*
 * ExtensionManifest.re
 *
 * Module to describing metadata about an extension
 */
open Oni_Core.Utility;
module Path = Rench.Path;

module Json = Oni_Core.Json;

[@deriving show]
type t = {
  name: string,
  version: string,
  author: string,
  displayName: option(LocalizedToken.t),
  description: option(string),
  // publisher: option(string),
  main: option(string),
  icon: option(string),
  categories: list(string),
  keywords: list(string),
  engines: string,
  activationEvents: list(string),
  extensionDependencies: list(string),
  extensionPack: list(string),
  extensionKind: kind,
  contributes: ExtensionContributions.t,
  enableProposedApi: bool,
}

and kind =
  | Ui
  | Workspace;

module Decode = {
  open Json.Decode;

  let kind =
    string |> map(fun
    | "ui" => Ui
    | "workspace" => Workspace
    | _ => Ui
    );

  let author =
    one_of([
      ("string", string),
      ("object", field("name", string)),
    ]);

  let engine =
    field("vscode", string);

  let manifest =
    field("name", string) >>= name =>
    field("version", string) >>= version =>
    one_of([
      ("author", field("author", author)),
      ("publisher", field("publisher", string)),
      ("default", succeed("Unknown Author")),
    ]) >>= author =>
    field_opt("displayName", LocalizedToken.decode) >>= displayName =>
    field_opt("description", string) >>= description =>
    field_opt("main", string) >>= main =>
    field_opt("icon", string) >>= icon =>
    field_opt("categories", list(string)) |> default([]) >>= categories =>
    field_opt("keywords", list(string)) |> default([]) >>= keywords =>
    field("engines", engine) >>= engines =>
    field_opt("activationEvents", list(string)) |> default([]) >>= activationEvents =>
    field_opt("extensionDependencies", list(string)) |> default([]) >>= extensionDependencies =>
    field_opt("extensionPack", list(string)) |> default([]) >>= extensionPack =>
    field_opt("extensionKind", kind) |> default(Ui) >>= extensionKind =>
    field("contributes", ExtensionContributions.decode) >>= contributes =>
    field_opt("enableProposedApi", bool) |> default(false) >>= enableProposedApi =>
    succeed({
      name, version, author, displayName, description, main, icon,
      categories, keywords, engines, activationEvents, extensionDependencies,
      extensionPack, extensionKind, contributes, enableProposedApi
    });
};

module Encode = {
  let kind =
    Json.Encode.(fun
    | Ui => string("ui")
    | Workspace => string("workspace")
    );
};

let decode = Decode.manifest;

let getDisplayName = (manifest: t) => {
  manifest.displayName
  |> Option.map(tok => LocalizedToken.to_string(tok))
  |> Option.value(~default=manifest.name);
};

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