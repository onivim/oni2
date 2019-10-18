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
};

let default: t = {
  meet: None,
  completions: [{
    completionLabel: "error",
    completionKind: CompletionKind.Method,
    completionDetail: Some("() => ()"),
  }, {
    completionLabel: "error",
    completionKind: CompletionKind.Method,
    completionDetail: None,
  }, {
    completionLabel: "error",
    completionKind: CompletionKind.Method,
    completionDetail: None,
  }],
}
