/*
 * HoverCollector.re
 *
 * This module 'collects' the information we want to show in the Hover UI,
 * based on the current state
 */

type hoverInfo = {diagnostics: list(Diagnostic.t)};

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
