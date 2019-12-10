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
    |> Utility.List.filter_map(_identity);

  let completorCount = ref(List.length(promises));
  let completions = ref([]);

  let (promise, waker) = Lwt.task();

  let decrement = () => {
    decr(completorCount);
    Log.info(
      "Decrementing - completor count now: " ++ string_of_int(completorCount^),
    );

    if (completorCount^ == 0) {
      // We're done!
      Lwt.wakeup(waker, completions^);
    };
  };

  List.iter(
    promise => {
      Lwt.on_success(
        promise,
        items => {
          completions := items @ completions^;
          decrement();
        },
      );

      Lwt.on_failure(
        promise,
        exn => {
          Log.error(
            "Error retrieving completions: " ++ Printexc.to_string(exn),
          );
          decrement();
        },
      );
    },
    promises,
  );

  promise;
};

let registerCompletionProvider =
    (completionProvider: CompletionProvider.t, v: t) => {
  completionProviders: [completionProvider, ...v.completionProviders],
};

let getCompletionProviderCount = (lf: t) =>
  List.length(lf.completionProviders);
