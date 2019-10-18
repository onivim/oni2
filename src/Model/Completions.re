/*
 * Completions.re
 *
 * This module is responsible for managing completion state
 */

open Oni_Core.Types;
open Oni_Extensions;

type t = {
  // The last completion meet we found
  meet: option(Actions.completionMeet),
  completions: list(Actions.completionItem),
  filteredCompletions: list(Actions.completionItem),
  filter: option(string),
  selected: int,
};

let default: t = {
  meet: None,
  selected: 0,
  filter: None,
  filteredCompletions: [],
  completions: [{
    completionLabel: "log",
    completionKind: CompletionKind.Method,
    completionDetail: Some("() => ()"),
  }, {
    completionLabel: "warn",
    completionKind: CompletionKind.Method,
    completionDetail: None,
  }, {
    completionLabel: "error",
    completionKind: CompletionKind.Method,
    completionDetail: None,
  }],
}

let endCompletions = (v: t) => {
  ...v,
  meet: None,
  filter: None,
  completions: [],
};

let startCompletions = (meet: Actions.completionMeet, v: t) => {
  ...v,
  meet: Some(meet),
  completions: [],
};

let _applyFilter = (filter: option(string), items: list(Actions.completionItem)) => {
  switch (filter) {
  | None => items
  | Some(filter) =>
  let re = Str.regexp_string(filter);
  List.filter((item: Actions.completionItem) => {
    switch (Str.search_forward(re, item.completionLabel)) {
    | exception Not_found => false
    | _ => true
    }
  }, items);
  }
};

let filter = (filter: string, v: t) => {
  ...v,
  filter: Some(filter),
  filteredCompletions: _applyFilter(Some(filter), v.completions)
};

let setItems = (items: list(Actions.completionItem), v: t) => {
  ...v,
  completions: items,
  filteredCompletions: _applyFilter(v.filter, items),
};

let reducer = (action: Actions.t, v: t) => {
  switch (action) {
  | Actions.CompletionStart(meet) => startCompletions(meet, v)
  | Actions.CompletionSetItems(_meet, items) => setItems(items, v)
  | Actions.CompletionBaseChanged(base) => filter(base, v)
  | Actions.CompletionEnd => endCompletions(v)
  | _ => v
  }
}
