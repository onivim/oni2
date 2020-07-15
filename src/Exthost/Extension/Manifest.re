/*
 * Manifest.re
 *
 * Module to describing metadata about an extension
 */
open Rench;

open Oni_Core;

module Kind = {
  [@deriving show]
  type t =
    | Ui
    | Workspace;

  let toString =
    fun
    | Ui => "ui"
    | Workspace => "workspace";

  module Decode = {
    open Json.Decode;

    let string =
      string
      |> map(
           fun
           | "ui" => Ui
           | "workspace" => Workspace
           | _ => Ui,
         );
  };

  let decode =
    Json.Decode.(
      one_of([
        ("single string", Decode.string |> map(kind => [kind])),
        ("list", list(Decode.string)),
      ])
    );

  let%test_module "json parsing" =
    (module
     {
       let parse = str =>
         str
         |> Yojson.Safe.from_string
         |> Json.Decode.decode_value(decode)
         |> Result.get_ok;

       let%test "json string" = {
         parse({|"ui"|}) == [Ui];
       };
       let%test "json list" = {
         parse({|["ui", "workspace"]|}) == [Ui, Workspace];
       };
     });
};

[@deriving show]
type t = {
  name: string,
  version: string,
  author: string,
  displayName: option(LocalizedToken.t),
  description: option(string),
  publisher: option(string),
  main: option(string),
  icon: option(string),
  categories: list(string),
  keywords: list(string),
  engines: string,
  activationEvents: list(string),
  extensionDependencies: list(string),
  extensionPack: list(string),
  extensionKind: list(Kind.t),
  contributes: Contributions.t,
  enableProposedApi: bool,
};

let identifier = manifest => {
  switch (manifest.publisher) {
  | Some(publisher) => publisher ++ "." ++ manifest.name
  | None => manifest.name
  };
};

let displayName = ({displayName, _}) => {
  displayName |> Option.map(LocalizedToken.toString);
};

module Decode = {
  open Json.Decode;

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
          publisher: field.optional("publisher", string),
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
          extensionKind:
            field.withDefault("extensionKind", [Kind.Ui], Kind.decode),
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
