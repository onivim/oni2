/*
 * Completions.re
 *
 * This module is responsible for managing completion state
 */

open Oni_Core;

module Option = Utility.Option;
module IndexEx = Utility.IndexEx;
module Zed_utf8 = Oni_Core.ZedBundled;

type filteredCompletion = Filter.result(CompletionItem.t);

type t = {
  // The last completion meet we found
  meet: option(CompletionMeet.t),
  all: list(CompletionItem.t),
  filtered: array(filteredCompletion),
  focused: option(int),
};

module Internal = {
  let filterItems = (maybeMeet, items) => {
    switch (maybeMeet) {
    | None =>
      items |> List.map(item => Filter.{item, highlight: []}) |> Array.of_list

    | Some((meet: CompletionMeet.t)) =>
      open CompletionItem;

      let query = Zed_utf8.explode(meet.base);

      let toString = (item, ~shouldLower) =>
        shouldLower ? String.lowercase_ascii(item.label) : item.label;

      items
      |> List.filter(item => Filter.fuzzyMatches(query, item.label))
      |> Filter.rank(meet.base, toString)
      |> Array.of_list;
    };
  };

  let ensureValidFocus = model => {
    ...model,
    focused:
      if (model.filtered == [||]) {
        None;
      } else {
        switch (model.focused) {
        | None => Some(0)
        | Some(index) =>
          index
          |> Utility.clamp(~lo=0, ~hi=Array.length(model.filtered) - 1)
          |> Option.some
        };
      },
  };
};

let initial = {meet: None, focused: None, filtered: [||], all: []};

let isActive = model => model.meet != None && model.filtered != [||];

let setMeet = (meet, model) =>
  Internal.ensureValidFocus({
    ...model,
    meet: Some(meet),
    filtered: Internal.filterItems(Some(meet), model.all),
  });

let addItems = (items, model) => {
  let all = List.concat([items, model.all]);
  let filtered = Internal.filterItems(model.meet, all);

  Internal.ensureValidFocus({...model, all, filtered});
};

let focusPrevious = model => {
  ...model,
  focused:
    IndexEx.prevRollOverOpt(
      model.focused,
      ~last=Array.length(model.filtered) - 1,
    ),
};

let focusNext = model => {
  ...model,
  focused:
    IndexEx.nextRollOverOpt(
      model.focused,
      ~last=Array.length(model.filtered) - 1,
    ),
};

let toString = model => {
  let filter =
    switch (model.meet) {
    | Some((meet: CompletionMeet.t)) => meet.base
    | None => "(None)"
    };
  Printf.sprintf(
    "Completions - meet: %s filter: %s",
    Option.toString(CompletionMeet.toString, model.meet),
    filter,
  );
};
