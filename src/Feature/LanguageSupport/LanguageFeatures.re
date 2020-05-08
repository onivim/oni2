/*
 * LanguageFeatures.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 */

open EditorCoreTypes;
open Oni_Core;
open Utility;

module SymbolKind = Exthost.SymbolKind;
module LocationWithUri = Exthost.Location;

let joinAll: list(Lwt.t(list('a))) => Lwt.t(list('a)) =
  promises => LwtEx.all((acc, curr) => acc @ curr, promises);

module CompletionProvider =
  LanguageFeature.Make({
    type params = (Buffer.t, CompletionMeet.t, Location.t);
    type response = list(CompletionItem.t);

    let namespace = "Oni2.CompletionProvider";
    let aggregate = joinAll;
  });

module DefinitionResult = {
  type t = {
    uri: Uri.t,
    location: Location.t,
    originRange: option(Range.t),
  };

  let create = (~originRange=None, ~uri, ~location) => {
    uri,
    location,
    originRange,
  };

  let toString = def =>
    Printf.sprintf(
      "Definition - uri: %s position: %s originRange: %s",
      Uri.toString(def.uri),
      Location.show(def.location),
      def.originRange |> OptionEx.toString(Range.show),
    );
};

module DefinitionProvider =
  LanguageFeature.Make({
    type params = (Buffer.t, Location.t);
    type response = DefinitionResult.t;

    let namespace = "Oni2.DefinitionProvider";
    let aggregate = Lwt.choose;
  });

module DocumentSymbolProvider =
  LanguageFeature.Make({
    type params = Buffer.t;
    type response = list(Exthost.DocumentSymbol.t);

    let namespace = "Oni2.DocumentSymbolProvider";
    let aggregate = joinAll;
  });

module DocumentHighlightResult = {
  // TODO: Add highlight kind
  type t = list(Range.t);

  let create = (~ranges) => ranges;
};

module DocumentHighlightProvider =
  LanguageFeature.Make({
    type params = (Buffer.t, Location.t);
    type response = list(Range.t);

    let namespace = "Oni2.DocumentHighlightProvider";
    let aggregate = joinAll;
  });

module FindAllReferencesProvider =
  LanguageFeature.Make({
    type params = (Buffer.t, Location.t);
    type response = list(Exthost.Location.t);

    let namespace = "Oni2.FindAllReferencesProvider";
    let aggregate = joinAll;
  });

[@deriving show({with_path: false})]
type action =
  | DocumentSymbolProviderAvailable(
      string,
      [@opaque] DocumentSymbolProvider.t,
    )
  | CompletionProviderAvailable(string, [@opaque] CompletionProvider.t)
  | DefinitionProviderAvailable(string, [@opaque] DefinitionProvider.t)
  | DocumentHighlightProviderAvailable(
      string,
      [@opaque] DocumentHighlightProvider.t,
    )
  | FindAllReferencesProviderAvailable(
      string,
      [@opaque] FindAllReferencesProvider.t,
    );

type t = {
  completionProviders: CompletionProvider.providers,
  definitionProviders: DefinitionProvider.providers,
  documentHighlightProviders: DocumentHighlightProvider.providers,
  documentSymbolProviders: DocumentSymbolProvider.providers,
  findAllReferencesProviders: FindAllReferencesProvider.providers,
};

let empty = {
  completionProviders: [],
  definitionProviders: [],
  documentHighlightProviders: [],
  documentSymbolProviders: [],
  findAllReferencesProviders: [],
};

let requestDocumentSymbol = (~buffer: Buffer.t, lf: t) => {
  DocumentSymbolProvider.request(buffer, lf.documentSymbolProviders);
};

let requestDefinition = (~buffer: Buffer.t, ~location: Location.t, lf: t) => {
  lf.definitionProviders |> DefinitionProvider.request((buffer, location));
};
let requestDocumentHighlights =
    (~buffer: Buffer.t, ~location: Location.t, lf: t) => {
  lf.documentHighlightProviders
  |> DocumentHighlightProvider.request((buffer, location));
};

let requestCompletions = (~buffer: Buffer.t, ~meet: CompletionMeet.t, lf: t) => {
  lf.completionProviders
  |> CompletionProvider.request((buffer, meet, meet.location));
};

let requestFindAllReferences =
    (~buffer: Buffer.t, ~location: Location.t, lf: t) => {
  lf.findAllReferencesProviders
  |> FindAllReferencesProvider.request((buffer, location));
};

let registerCompletionProvider = (~id, ~provider: CompletionProvider.t, lf: t) => {
  ...lf,
  completionProviders:
    lf.completionProviders |> CompletionProvider.register(~id, provider),
};

let registerDefinitionProvider = (~id, ~provider: DefinitionProvider.t, lf: t) => {
  ...lf,
  definitionProviders:
    DefinitionProvider.register(~id, provider, lf.definitionProviders),
};

let registerDocumentHighlightProvider =
    (~id, ~provider: DocumentHighlightProvider.t, lf: t) => {
  ...lf,
  documentHighlightProviders:
    lf.documentHighlightProviders
    |> DocumentHighlightProvider.register(~id, provider),
};

let registerDocumentSymbolProvider =
    (~id, ~provider: DocumentSymbolProvider.t, lf: t) => {
  ...lf,
  documentSymbolProviders:
    lf.documentSymbolProviders
    |> DocumentSymbolProvider.register(~id, provider),
};

let registerFindAllReferencesProvider =
    (~id, ~provider: FindAllReferencesProvider.t, lf: t) => {
  ...lf,
  findAllReferencesProviders:
    lf.findAllReferencesProviders
    |> FindAllReferencesProvider.register(~id, provider),
};

let getCompletionProviders = (lf: t) =>
  lf.completionProviders |> CompletionProvider.get;

let getDefinitionProviders = (lf: t) => {
  lf.definitionProviders |> DefinitionProvider.get;
};

let getDocumentHighlightProviders = (lf: t) => {
  lf.documentHighlightProviders |> DocumentHighlightProvider.get;
};

let getDocumentSymbolProviders = lf =>
  lf.documentSymbolProviders |> DocumentSymbolProvider.get;

let getFindAllReferencesProviders = lf =>
  lf.findAllReferencesProviders |> FindAllReferencesProvider.get;
