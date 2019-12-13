/*
 * LanguageFeatures.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 */

open EditorCoreTypes;
open Oni_Core;

let joinAll = Utility.LwtUtil.all((acc, curr) => acc @ curr, promises);

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
      def.originRange |> Utility.Option.toString(Range.show),
    );
};

module DefinitionProvider =
  LanguageFeature.Make({
    type params = (Buffer.t, Location.t);
    type response = DefinitionResult.t;

    let namespace = "Oni2.DefinitionProvider";
    let aggregate = Lwt.choose;
  });

module SymbolKind = {
  type t =
    | File
    | Module
    | Namespace
    | Package
    | Class
    | Method
    | Property
    | Field
    | Constructor
    | Enum
    | Interface
    | Function
    | Variable
    | Constant
    | String
    | Number
    | Boolean
    | Array
    | Object
    | Key
    | Null
    | EnumMember
    | Struct
    | Event
    | Operator
    | TypeParameter;
};

module DocumentSymbol = {
  type t = {
    name: string,
    detail: string,
    kind: SymbolKind.t,
    //TODO: containerName?
    range: Range.t,
    //TODO: selectionRange?
    children: list(t),
  };

  let create = (~children=[], ~name, ~detail, ~kind, ~range) => {
    name,
    detail,
    kind,
    range,
    children,
  };
};

module DocumentSymbolProvider =
  LanguageFeature.Make({
    type params = Buffer.t;
    type response = list(DocumentSymbol.t);

    let namespace = "Oni2.DocumentSymbolProvider";
    let aggregate = Lwt.choose;
  });

module DocumentHighlightResult = {
  // TODO: Add highlight kind
  type t = list(Range.t);

  let create = (~ranges) => ranges;
};

module DocumentHighlightProvider =
  LanguageFeature.Make({
    type params = (Buffer.t, Location.t);
    type result = DocumentHighlightResult.t;

    let namespace = "Oni2.DocumentHighlightProvider";
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
    );

type t = {
  completionProviders: CompletionProvider.providers,
  definitionProviders: DefinitionProvider.providers,
  documentHighlightProviders: DocumentHighlightProvider.providers,
  documentSymbolProviders: DocumentSymbolProvider.providers,
};

let empty = {
  completionProviders: [],
  definitionProviders: [],
  documentHighlightProviders: [],
  documentSymbolProviders: [],
};

let requestDocumentSymbol = (~buffer: Buffer.t, lf: t) => {
  DocumentSymbolProvider.request(buffer, lf.documentSymbolProviders);
};

let requestDefinition = (~buffer: Buffer.t, ~location: Location.t, lf: t) => {
  lf.definitionProviders |> DefinitionProvider.request((buffer, location));
};
let requestDocumentHighlights =
    (~buffer: Buffer.t, ~location: Location.t, lf: t) => {
  lf.documentHighlightsProviders
  |> DocumentHighlightsProvider.request((buffer, location));
};

let requestCompletions =
    (~buffer: Buffer.t, ~meet: CompletionMeet.t, ~location: Location.t, lf: t) => {
  lf.completionProviders
  |> CompletionProvider.request((buffer, meet, location));
};

let registerCompletionProvider = (~id, ~provider: CompletionProvider.t, lf: t) => {
  lf.completionProvders |> CompletionProvider.register(~id, provdier);
};

let registerDefinitionProvider = (~id, ~provider: DefinitionProvider.t, lf: t) =>
  DefinitionProvider.register(~id, provider, lf.definitionProviders);

let registerDocumentHighlightProvider =
    (~id, ~provider: DocumentHighlightProvider.t, lf: t) => {
  lf.documentHighlightProviders
  |> DocumentHighlightsProvider.register(~id, provider);
};

let registerDocumentSymbolProvider =
    (~id, ~provider: DocumentSymbolProvider.t, lf: t) =>
  lf.documentSymbolProviders |> DocumentSymbolProvider.register(~id, provider);

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
