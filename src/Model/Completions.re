/*
 * Completions.re
 *
 * This module is responsible for managing completion state
 */

open Oni_Core;

module Zed_utf8 = Oni_Core.ZedBundled;

type filteredCompletion = Filter.result(Actions.completionItem);

type t = {
  // The last completion meet we found
  meet: option(Actions.completionMeet),
  completions: list(Actions.completionItem),
  filteredCompletions: list(filteredCompletion),
  filter: option(string),
  selected: option(int),
};

let default: t = {
  meet: None,
  selected: None,
  filter: None,
  filteredCompletions: [],
  completions: [],
};

let isActive = (v: t) => {
  v.meet != None && v.filteredCompletions != [];
};

let endCompletions = (_v: t) => {
  default;
};

let startCompletions = (meet: Actions.completionMeet, v: t) => {
  {
    ...v,
    meet: Some(meet),
    completions: default.completions,
    filteredCompletions: default.filteredCompletions,
  };
};

let getMeet = (v: t) => v.meet;

let getBestCompletion = (v: t) => {
  List.nth_opt(v.filteredCompletions, 0);
};

let _toFilterResult = (items: list(Actions.completionItem)) => {
  Filter.(items |> List.map(item => {item, highlight: []}));
};

let getCompletions = (v: t) => v.filteredCompletions;

let _applyFilter =
    (filter: option(string), items: list(Actions.completionItem)) => {
  switch (filter) {
  | None => items |> _toFilterResult
  | Some(filter) =>
    open Actions;

    let query = Zed_utf8.explode(filter);

    let toString = (item, ~shouldLower) =>
      if (shouldLower) {
        item.completionLabel |> String.lowercase_ascii;
      } else {
        item.completionLabel;
      };

    items
    |> List.filter(item => Filter.fuzzyMatches(query, item.completionLabel))
    |> Filter.rank(filter, toString);
  };
};

let filter = (filter: string, v: t) => {
  {
    ...v,
    filter: Some(filter),
    filteredCompletions:
      _applyFilter(Some(filter), v.completions) |> Utility.firstk(5),
  };
};

let addItems = (items: list(Actions.completionItem), v: t) => {
  let newItems = List.concat([items, v.completions]);
  {
    ...v,
    completions: newItems,
    filteredCompletions:
      _applyFilter(v.filter, newItems) |> Utility.firstk(5),
  };
};

let reduce = (v: t, action: Actions.t) => {
  let newV =
    switch (action) {
    | Actions.ChangeMode(mode) when mode != Vim.Types.Insert =>
      endCompletions(v)
    | Actions.CompletionStart(meet) => startCompletions(meet, v)
    | Actions.CompletionAddItems(_meet, items) => addItems(items, v)
    | Actions.CompletionBaseChanged(base) => filter(base, v)
    | Actions.CompletionEnd => endCompletions(v)
    | _ => v
    };

  if (isActive(newV)) {
    switch (action) {
    | Actions.Command("selectNextSuggestion") =>
      // TODO
      newV
    | Actions.Command("selectPrevSuggestion") =>
      // TODO
      newV
    | _ => newV
    };
  } else {
    newV;
  };
};
