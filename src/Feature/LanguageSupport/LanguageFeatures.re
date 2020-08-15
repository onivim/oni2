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

module DocumentSymbolProvider =
  LanguageFeature.Make({
    type params = Buffer.t;
    type response = list(Exthost.DocumentSymbol.t);

    let namespace = "Oni2.DocumentSymbolProvider";
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
  | FindAllReferencesProviderAvailable(
      string,
      [@opaque] FindAllReferencesProvider.t,
    );

type t = {
  documentSymbolProviders: DocumentSymbolProvider.providers,
  findAllReferencesProviders: FindAllReferencesProvider.providers,
};

let empty = {documentSymbolProviders: [], findAllReferencesProviders: []};

let requestDocumentSymbol = (~buffer: Buffer.t, lf: t) => {
  DocumentSymbolProvider.request(buffer, lf.documentSymbolProviders);
};

let requestFindAllReferences =
    (~buffer: Buffer.t, ~location: Location.t, lf: t) => {
  lf.findAllReferencesProviders
  |> FindAllReferencesProvider.request((buffer, location));
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

let getDocumentSymbolProviders = lf =>
  lf.documentSymbolProviders |> DocumentSymbolProvider.get;

let getFindAllReferencesProviders = lf =>
  lf.findAllReferencesProviders |> FindAllReferencesProvider.get;
