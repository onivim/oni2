/*
 * LanguageFeatures.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 */

open Oni_Core;

module CompletionProvider = {
  type t =
    (Buffer.t, CompletionMeet.t, Position.t) =>
    option(Lwt.t(list(CompletionItem.t)));
};

type t = {completionProviders: list(CompletionProvider.t)};

let empty = {completionProviders: []};

let _identity = v => v;

let requestCompletions =
    (~buffer: Buffer.t, ~meet: CompletionMeet.t, ~position: Position.t, lf: t) => {
  let promises =
    lf.completionProviders
    |> List.map(suggestor => suggestor(buffer, meet, position))
    |> Utility.List.filter_map(_identity)
    // TODO: Handle multiple providers..
    |> Lwt.choose;

  promises;
};

let registerCompletionProvider =
    (completionProvider: CompletionProvider.t, v: t) => {
  completionProviders: [completionProvider, ...v.completionProviders],
};
