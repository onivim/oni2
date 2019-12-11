/*
 * LanguageFeatures.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 */

open Oni_Core;

module CompletionLog = (val Log.withNamespace("Oni2.CompletionProvider"));

module CompletionProvider = {
  type t =
    (Buffer.t, CompletionMeet.t, Position.t) =>
    option(Lwt.t(list(CompletionItem.t)));

  type info = {
    provider: t,
    id: string,
  };
};

type t = {completionProviders: list(CompletionProvider.info)};

let empty = {completionProviders: []};

let _identity = v => v;

let requestCompletions =
    (~buffer: Buffer.t, ~meet: CompletionMeet.t, ~position: Position.t, lf: t) => {
  let promises =
    lf.completionProviders
    |> List.map((CompletionProvider.{id, provider}) => {
         let result = provider(buffer, meet, position);
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

let registerCompletionProvider = (~id, ~provider: CompletionProvider.t, v: t) => {
  completionProviders: [{id, provider}, ...v.completionProviders],
};

let getCompletionProviders = (lf: t) =>
  lf.completionProviders |> List.map((CompletionProvider.{id, _}) => id);
