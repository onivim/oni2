/**
 * Range.rei
 *
 * Module dealing with Range (tuples of positions)
 */

open Types;

[@deriving show({with_path: false})]
type t = {
  startPosition: Position.t,
  endPosition: Position.t,
};

let createFromPositions:
  (~startPosition: Position.t, ~endPosition: Position.t, unit) => t;

let create:
  (
    ~startLine: Index.t,
    ~startCharacter: Index.t,
    ~endLine: Index.t,
    ~endCharacter: Index.t,
    unit
  ) =>
  t;

let zero: t;

/*
 * explode(range, measure) takes a Range.t and a measurement function (int => int),
 * and expands a multiple-line range into a list of ranges, where there there is
 * a single range per-line.
 *
 * If the input range is a single line, a single item list with the input range
 * will be returned.
 */
let explode: (t, int => int) => list(t);
