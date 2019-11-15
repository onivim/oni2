/*
 * LanguageFeatures.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 */

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

type t = {suggestProviders: list(SuggestProvider.t)};

let create = () => {suggestProviders: []};

let getSuggestProviders = (fileType: string, v: t) => {
  let filter = (sp: SuggestProvider.t) =>
    DocumentSelector.matches(sp.selector, fileType);
  List.filter(filter, v.suggestProviders);
};

let registerSuggestProvider = (suggestProvider: SuggestProvider.t, v: t) => {
  suggestProviders: [suggestProvider, ...v.suggestProviders],
};
