/*
 * LanguageFeatures.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 */

open EditorCoreTypes;
open Oni_Core;

module Make =
       (
         Provider: {
           type params;
           type response;

           let namespace: string;

           let aggregate: list(Lwt.t(response)) => Lwt.t(response);
         },
       ) => {
  module Log = (val Log.withNamespace(Provider.namespace));

  module Params = {
    type t = Provider.params;
  };
  type response = Provider.response;

  type t = Params.t => option(Lwt.t(response));

  type info = {
    provider: t,
    id: string,
  };

  type providers = list(info);

  let register = (~id: string, provider: t, providers: providers) => {
    [{id, provider}, ...providers];
  };

  let get = (providers: providers) => {
    providers |> List.map(({id, _}) => id);
  };

  let request = (params: Params.t, providers: providers) => {
    let promises =
      providers
      |> List.map(({id, provider}) => {
           let result = provider(params);
           switch (result) {
           | Some(_) => Log.infof(m => m("Querying provider: %s", id))
           | None => Log.infof(m => m("Provider skipped: %s", id))
           };
           result;
         })
      |> Utility.Option.values;

    Provider.aggregate(promises);
  };
};

module CompletionLog = (val Log.withNamespace("Oni2.CompletionProvider"));

module CompletionProvider = {
  type t =
    (Buffer.t, CompletionMeet.t, Location.t) =>
    option(Lwt.t(list(CompletionItem.t)));

  type info = {
    provider: t,
    id: string,
  };
};

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
  Make({
    type params = (Buffer.t, Location.t);
    type response = DefinitionResult.t;

    let namespace = "DefinitionProvider";
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
  Make({
    type params = Buffer.t;
    type response = list(DocumentSymbol.t);

    let namespace = "DocumentSymbolProvider";
    let aggregate = Lwt.choose;
  });

module DocumentHighlightResult = {
  // TODO: Add highlight kind
  type t = list(Range.t);

  let create = (~ranges) => ranges;
};

module DocumentHighlightProvider = {
  type t =
    (Buffer.t, Location.t) => option(Lwt.t(DocumentHighlightResult.t));

  type info = {
    id: string,
    provider: t,
  };
};

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
  completionProviders: list(CompletionProvider.info),
  definitionProviders: DefinitionProvider.providers,
  documentHighlightProviders: list(DocumentHighlightProvider.info),
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
  let promises =
    lf.documentHighlightProviders
    |> List.map((DocumentHighlightProvider.{provider, _}) =>
         provider(buffer, location)
       )
    |> Utility.Option.values;

  let join = (accCompletions, currCompletions) =>
    accCompletions @ currCompletions;

  Utility.LwtUtil.all(join, promises);
};

let requestCompletions =
    (~buffer: Buffer.t, ~meet: CompletionMeet.t, ~location: Location.t, lf: t) => {
  let promises =
    lf.completionProviders
    |> List.map((CompletionProvider.{id, provider}) => {
         let result = provider(buffer, meet, location);
         switch (result) {
         | Some(_) =>
           CompletionLog.infof(m => m("Querying completion provider: %s", id))
         | None =>
           CompletionLog.infof(m => m("Completion provider skipped: %s", id))
         };
         result;
       })
    |> Utility.Option.values;

  let join = (accCompletions, currCompletions) =>
    accCompletions @ currCompletions;

  Utility.LwtUtil.all(join, promises);
};

let registerCompletionProvider = (~id, ~provider: CompletionProvider.t, lf: t) => {
  ...lf,
  completionProviders: [{id, provider}, ...lf.completionProviders],
};

let registerDefinitionProvider = (~id, ~provider: DefinitionProvider.t, lf: t) =>
  DefinitionProvider.register(~id, provider, lf.definitionProviders);

let registerDocumentHighlightProvider =
    (~id, ~provider: DocumentHighlightProvider.t, lf: t) => {
  ...lf,
  documentHighlightProviders: [
    {id, provider},
    ...lf.documentHighlightProviders,
  ],
};

let registerDocumentSymbolProvider =
    (~id, ~provider: DocumentSymbolProvider.t, lf: t) =>
  lf.documentSymbolProviders |> DocumentSymbolProvider.register(~id, provider);

let getCompletionProviders = (lf: t) =>
  lf.completionProviders |> List.map((CompletionProvider.{id, _}) => id);

let getDefinitionProviders = (lf: t) => {
  DefinitionProvider.get(lf.definitionProviders);
};

let getDocumentHighlightProviders = (lf: t) => {
  lf.documentHighlightProviders
  |> List.map((DocumentHighlightProvider.{id, _}) => id);
};

let getDocumentSymbolProviders = lf =>
  lf.documentSymbolProviders |> DocumentSymbolProvider.get;
