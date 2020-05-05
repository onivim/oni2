/*
 * Manifest.re
 *
 * Module to describing metadata about an extension
 */
open Rench;

open Oni_Core;

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
  contributes: Contributions.t,
  enableProposedApi: bool,
}

and kind =
  | Ui
  | Workspace;

module Decode = {
  open Json.Decode;

  let kind =
    string
    |> map(
         fun
         | "ui" => Ui
         | "workspace" => Workspace
         | _ => Ui,
       );

  let author =
    one_of([("string", string), ("object", field("name", string))]);

  let engine = field("vscode", string);

  let manifest =
    Json.Decode.(
      obj(({field, whatever, _}) =>
        {
          name: field.required("name", string),
          version: field.required("version", string),
          author:
            whatever(
              one_of([
                ("author", field.monadic("author", author)),
                ("publisher", field.monadic("publisher", string)),
                ("default", succeed("Unknown Author")),
              ]),
            ),
          displayName: field.optional("displayName", LocalizedToken.decode),
          description: field.optional("description", string),
          main: field.optional("main", string),
          icon: field.optional("icon", string),
          categories: field.withDefault("categories", [], list(string)),
          keywords: field.withDefault("keywords", [], list(string)),
          engines: field.required("engines", engine),
          activationEvents:
            field.withDefault("activationEvents", [], list(string)),
          extensionDependencies:
            field.withDefault("extensionDependencies", [], list(string)),
          extensionPack:
            field.withDefault("extensionPack", [], list(string)),
          extensionKind: field.withDefault("extensionKind", Ui, kind),
          contributes:
            field.withDefault(
              "contributes",
              Contributions.default,
              Contributions.decode,
            ),
          enableProposedApi:
            field.withDefault("enableProposedApi", false, bool),
        }
      )
    );
};

module Encode = {
  let kind =
    Json.Encode.(
      fun
      | Ui => string("ui")
      | Workspace => string("workspace")
    );
};

let decode = Decode.manifest;

let getDisplayName = (manifest: t) => {
  manifest.displayName
  |> Option.map(tok => LocalizedToken.toString(tok))
  |> Option.value(~default=manifest.name);
};

let remapPaths = (rootPath: string, manifest: t) => {
  ...manifest,
  //main: Option.map(Path.join(rootPath), manifest.main),
  icon: Option.map(Path.join(rootPath), manifest.icon),
  contributes: Contributions.remapPaths(rootPath, manifest.contributes),
};

let updateName = (nameSetter, manifest: t) => {
  ...manifest,
  name: nameSetter(manifest.name),
};

let localize = (loc: LocalizationDictionary.t, manifest: t) => {
  ...manifest,
  displayName:
    Option.map(LocalizedToken.localize(loc), manifest.displayName),
  contributes: Contributions.localize(loc, manifest.contributes),
};
