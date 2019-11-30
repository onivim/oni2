/*
 * BufferHighlights.rei
 */

module Time = Revery.Time;

open Oni_Core;
open Oni_Core.Types;

type highlightType =
  | Insert
  | Delete
  | Yank;

type highlight = {
  range: Range.t,
  highlightType,
};

type t;

let getActive: (Time.t, t) => list(highlight);

let add: (Time.t, list(highlight), t) => t;

let empty: t;
