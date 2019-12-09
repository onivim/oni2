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

module DefinitionProvider = {
  type t = (Buffer.t, Position.t) => option(Lwt.t(Position.t));
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

let getDefinition = (buffer: Buffer.t, pos: Position.t, lf: t) => {
  lf.definitionProviders
  |> List.map(df => df(buffer, pos))
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
