/*
 * Diagnostics.rei
 *
 * This module is responsible for tracking the state of 'diagnostics'
 * (usually errors or warnings) that we render in the buffer view
 * or minimap.
 */

open EditorCoreTypes;
open Oni_Core;

module Diagnostic: {
  type t = {
    range: CharacterRange.t,
    message: string,
  };

  let create: (~range: CharacterRange.t, ~message: string, unit) => t;

  /*
    [explode(buffer, diagnostic)] splits up a multi-line diagnostic into a diagnostic per-line
   */
  let explode: (Buffer.t, t) => list(t);

  let pp: (Format.formatter, t) => unit;
};

// MODEL

[@deriving show]
type msg;

module Msg: {
  let exthost: Exthost.Msg.Diagnostics.msg => msg;
  let diagnostics: (Uri.t, string, list(Diagnostic.t)) => msg;
  let clear: (~owner: string) => msg;
};

type model;

let initial: model;

// UPDATE

let update: (msg, model) => model;

/*
 * Change diagnostics for a buffer+diagnostic key pair
 */
let change: (model, Uri.t, string, list(Diagnostic.t)) => model;

/*
 * [clear(diagnostics, key)] removes diagnostics with the key named [key] across all buffers
 */
let clear: (model, string) => model;

/*
 * [count(diagnostics)] gets the total count of all diagnostics across buffers
 */
let count: model => int;

/*
 * Get all diagnostics for a buffer
 */
let getDiagnostics: (model, Buffer.t) => list(Diagnostic.t);
let getDiagnosticsAtPosition:
  (model, Buffer.t, CharacterPosition.t) => list(Diagnostic.t);
let getDiagnosticsMap: (model, Buffer.t) => IntMap.t(list(Diagnostic.t));

let getAllDiagnostics: model => list((Uri.t, Diagnostic.t));

module View: {
  let make:
    (
      ~onSelectFile: (
                       ~filePath: string,
                       ~position: EditorCoreTypes.CharacterPosition.t
                     ) =>
                     unit,
      ~diagnostics: model,
      ~theme: ColorTheme.Colors.t,
      ~uiFont: UiFont.t,
      ~editorFont: Service_Font.font,
      ~workingDirectory: string,
      unit
    ) =>
    Revery.UI.element;
};
