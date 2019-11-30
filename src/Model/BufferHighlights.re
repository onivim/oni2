/*
 * BufferHighlights.re
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

type internalHighlight = {
  range: Range.t,
  highlightType,
  time: Time.t,
};

type t = {highlights: list(internalHighlight)};

// Only report highlightgs within one second
let threshold = 1.0;

let _filterActive =
    (currentTime: Time.t, highlights: list(internalHighlight)) => {
  let filter = _ => {
    true;
  };
  /*let filter = (highlight) => {
     let delta = Time.(currentTime - highlight.time) |> Time.toFloatSeconds;
     delta < threshold
    } */

  highlights |> List.filter(filter);
};

let _internalToExternal: internalHighlight => highlight =
  internalHighlight => {
    range: internalHighlight.range,
    highlightType: internalHighlight.highlightType,
  };

let _externalToInternal: (Time.t, highlight) => internalHighlight =
  (time, internalHighlight) => {
    time,
    range: internalHighlight.range,
    highlightType: internalHighlight.highlightType,
  };

let getActive = (time, bufferHighlights) => {
  bufferHighlights.highlights
  |> _filterActive(time)
  |> List.map(_internalToExternal);
};

let add = (time, newHighlights, model) => {
  // First, filter out inactive highlights
  let currentHighlights = _filterActive(time, model.highlights);

  let newHighlights = newHighlights |> List.map(_externalToInternal(time));

  {highlights: List.concat([currentHighlights, newHighlights])};
};

let empty = {highlights: []};
