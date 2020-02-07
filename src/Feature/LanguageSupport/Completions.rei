open Oni_Core;

type filteredCompletion = Filter.result(CompletionItem.t);

type t =
  pri {
    // The last completion meet we found
    meet: option(CompletionMeet.t),
    all: list(CompletionItem.t),
    filtered: array(filteredCompletion),
    focused: option(int),
  };

let initial: t;

let isActive: t => bool;
let setMeet: (CompletionMeet.t, t) => t;
let addItems: (list(CompletionItem.t), t) => t;
let focusPrevious: t => t;
let focusNext: t => t;

let toString: t => string;
