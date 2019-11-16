/*
 * Diagnostic.re
 */

open Oni_Core;

type t = {
  range: Range.t,
  message: string,
};

let create = (~range: Range.t, ~message, ()) => {range, message};

let explode = (buffer: Buffer.t, v: t) => {
  let measure = Buffer.getLineLength(buffer);

  Range.explode(measure, v.range)
  |> List.map(range => create(~range, ~message=v.message, ()));
};
