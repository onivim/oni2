/*
 * LanguageFeatures.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 */

open EditorCoreTypes;
open Oni_Core;

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

module DefinitionProvider = {
  type t = (Buffer.t, Location.t) => option(Lwt.t(DefinitionResult.t));

  type info = {
    provider: t,
    id: string,
  };
};

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
  | CompletionProviderAvailable(string, [@opaque] CompletionProvider.t)
  | DefinitionProviderAvailable(string, [@opaque] DefinitionProvider.t)
  | DocumentHighlightProviderAvailable(
      string,
      [@opaque] DocumentHighlightProvider.t,
    );

type t = {
  completionProviders: list(CompletionProvider.info),
  definitionProviders: list(DefinitionProvider.info),
  documentHighlightProviders: list(DocumentHighlightProvider.info),
};

let empty = {
  completionProviders: [],
  definitionProviders: [],
  documentHighlightProviders: [],
};

let requestDefinition = (~buffer: Buffer.t, ~location: Location.t, lf: t) => {
  lf.definitionProviders
  |> List.map((DefinitionProvider.{provider, _}) =>
       provider(buffer, location)
     )
  |> Utility.Option.values
  |> Lwt.choose;
};
exception TestFailure;

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

let registerDefinitionProvider = (~id, ~provider: DefinitionProvider.t, lf: t) => {
  ...lf,
  definitionProviders: [{id, provider}, ...lf.definitionProviders],
};

let registerDocumentHighlightProvider =
    (~id, ~provider: DocumentHighlightProvider.t, lf: t) => {
  ...lf,
  documentHighlightProviders: [
    {id, provider},
    ...lf.documentHighlightProviders,
  ],
};

let getCompletionProviders = (lf: t) =>
  lf.completionProviders |> List.map((CompletionProvider.{id, _}) => id);

let getDefinitionProviders = (lf: t) => {
  lf.definitionProviders |> List.map((DefinitionProvider.{id, _}) => id);
};

let getDocumentHighlightProviders = (lf: t) => {
  lf.documentHighlightProviders
  |> List.map((DocumentHighlightProvider.{id, _}) => id);
};
