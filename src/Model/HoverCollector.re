/*
 * HoverCollector.re
 *
 * This module 'collects' the information we want to show in the Hover UI,
 * based on the current state
 */

open Oni_Core;
open Oni_Core.Types;

type hoverInfo = {diagnostics: list(Diagnostics.Diagnostic.t)};

type t = option(hoverInfo);

let empty: t = None;

let get = (state: State.t) => {
  switch (state.hover) {
  | None => None
  | Some(hover) =>
    let buffer = Buffers.getBuffer(hover.bufferId, state.buffers);

    switch (buffer) {
    | None => None
    | Some(buf) =>
      let diagnostics =
        Diagnostics.getDiagnosticsAtPosition(
          state.diagnostics,
          buf,
          hover.position,
        );

      switch (diagnostics) {
      | [] => None
      | _ => Some({diagnostics: diagnostics})
      };
    };
  };
};
