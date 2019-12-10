/*
 * LanguageFeatures.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 */

open Oni_Core;

module SuggestProvider = {
  type t = {
    id: int,
    selector: DocumentSelector.t,
    triggerCharacters: list(string),
    supportsResolve: bool,
  };

  let create = (~triggerCharacters=[], ~supportsResolve=false, ~selector, id) => {
    id,
    selector,
    triggerCharacters,
    supportsResolve,
  };
};

module Definition = {
  type t = {
    uri: Uri.t,
    position: Position.t,
  };

  let create = (~uri, ~position) => {uri, position};

  let toString = def =>
    Printf.sprintf(
      "Definition - uri: %s position: %s",
      Uri.toString(def.uri),
      Position.show(def.position),
    );
};

module DefinitionProvider = {
  type t = (Uri.t, Position.t) => option(Lwt.t(Definition.t));
};

type t = {
  suggestProviders: list(SuggestProvider.t),
  definitionProviders: list(DefinitionProvider.t),
};

let empty = {suggestProviders: [], definitionProviders: []};

let getSuggestProviders = (fileType: string, v: t) => {
  let filter = (sp: SuggestProvider.t) => {
    DocumentSelector.matches(sp.selector, fileType);
  };
  List.filter(filter, v.suggestProviders);
};

let getDefinition = (uri: Uri.t, pos: Position.t, lf: t) => {
  prerr_endline(
    "DEFINITION PROVIDER COUNT: "
    ++ string_of_int(List.length(lf.definitionProviders)),
  );
  lf.definitionProviders
  |> List.map(df => df(uri, pos))
  |> Utility.List.filter_map(v => v)
  |> Lwt.choose;
};

let registerSuggestProvider =
    (suggestProvider: SuggestProvider.t, languageFeatures: t) => {
  ...languageFeatures,
  suggestProviders: [suggestProvider, ...languageFeatures.suggestProviders],
};

let registerDefinitionProvider =
    (definitionProvider: DefinitionProvider.t, languageFeatures) => {
  ...languageFeatures,
  definitionProviders: [
    definitionProvider,
    ...languageFeatures.definitionProviders,
  ],
};
