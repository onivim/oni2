/*
 * Completions.re
 *
 * This module is responsible for managing completion state
 */

open Oni_Core;
module Option = Utility.Option;

module Zed_utf8 = Oni_Core.ZedBundled;

type filteredCompletion = Filter.result(CompletionItem.t);

type t = {
  // The last completion meet we found
  meet: option(CompletionMeet.t),
  all: list(CompletionItem.t),
  filtered: array(filteredCompletion),
  filter: option(string),
  focused: option(int),
};

let toString = model => {
  let filter =
    switch (model.filter) {
    | Some(filter) => filter
    | None => "(None)"
    };
  Printf.sprintf(
    "Completions - meet: %s filter: %s",
    Option.toString(CompletionMeet.toString, model.meet),
    filter,
  );
};

let initial = {
  meet: None,
  focused: None,
  filter: None,
  filtered: [||],
  all: [],
};

let isActive = model => model.meet != None && model.filtered != [||];

let filterItems = (filter, items) => {
  switch (filter) {
  | None =>
    items |> List.map(item => Filter.{item, highlight: []}) |> Array.of_list

  | Some(filter) =>
    open CompletionItem;

    let query = Zed_utf8.explode(filter);

    let toString = (item, ~shouldLower) =>
      shouldLower ? String.lowercase_ascii(item.label) : item.label;

    items
    |> List.filter(item => Filter.fuzzyMatches(query, item.label))
    |> Filter.rank(filter, toString)
    |> Array.of_list;
  };
};

let getMeet = model => model.meet;

let setFilter = (filter, model) => {
  ...model,
  filter: Some(filter),
  filtered: filterItems(Some(filter), model.all),
};

let reduce = (model, action: Actions.t) => {
  switch (action) {
  | ChangeMode(mode) when mode != Vim.Types.Insert => initial

  | CompletionStart(meet) => {
      ...model,
      meet: Some(meet),
      all: initial.all,
      filtered: initial.filtered,
    }

  | CompletionAddItems(_meet, items) =>
    let all = List.concat([items, model.all]);
    let filtered = filterItems(model.filter, all);
    {
      ...model,
      all,
      focused:
        model.focused == None && Array.length(filtered) > 0
          ? Some(0) : model.focused,
      filtered,
    };

  | CompletionBaseChanged(base) => setFilter(base, model)
  | CompletionEnd => initial

  | Command("selectNextSuggestion") => model
  | Command("selectPrevSuggestion") => model
  | _ => model
  };
};
