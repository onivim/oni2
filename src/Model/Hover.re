/*
 * Hover.re
 *
 * This module is responsible for the types and operations
 * for the 'Hover' view
 */

open Oni_Core;
open Oni_Core.Types;

type t = {
  diagnostics: list(Diagnostics.Diagnostic.t),
};

let get = (~bufferId, ~position: Position.t, state: State.t) => {
  let buffer = Buffers.getBuffer(bufferId, state.buffers);

  switch (buffer) {
  | None => None
  | Some(buf) => {
    let diagnostics = Diagnostics.getDiagnosticsAtPosition(state.diagnostics, buf, position);

    switch (diagnostics) {
    | [] => None
    | _ => Some({ diagnostics: diagnostics });
    }
  }
  }
};
