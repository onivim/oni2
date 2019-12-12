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
    position: Location.t,
  };

  let create = (~uri, ~position) => {uri, position};

  let toString = def =>
    Printf.sprintf(
      "Definition - uri: %s position: %s",
      Uri.toString(def.uri),
      Location.show(def.position),
    );
};

module DefinitionProvider = {
  type t = (Buffer.t, Location.t) => option(Lwt.t(DefinitionResult.t));

  type info = {
    provider: t,
    id: string,
  };
};

[@deriving show({with_path: false})]
type action =
  | CompletionProviderAvailable(string, [@opaque] CompletionProvider.t)
  | DefinitionProviderAvailable(string, [@opaque] DefinitionProvider.t);

type t = {
  completionProviders: list(CompletionProvider.info),
  definitionProviders: list(DefinitionProvider.info),
};

let empty = {completionProviders: [], definitionProviders: []};

let requestDefinition = (~buffer: Buffer.t, ~location: Location.t, lf: t) => {
  lf.definitionProviders
  |> List.map((DefinitionProvider.{provider, _}) =>
       provider(buffer, location)
     )
  |> Utility.Option.values
  |> Lwt.choose;
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

let getCompletionProviders = (lf: t) =>
  lf.completionProviders |> List.map((CompletionProvider.{id, _}) => id);

let getDefinitionProviders = (lf: t) => {
  lf.definitionProviders |> List.map((DefinitionProvider.{id, _}) => id);
};
