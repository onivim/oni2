/*
 * ColorMap.re
 *
 * A mapping of int -> colors
 */

open Revery;

[@deriving yojson({strict: false, exn: true})]
type raw = list(string);

type t = array(Color.t);

let create = () => [||];

let ofJson = (v: Yojson.Safe.json) => {
  raw_of_yojson_exn(v) |> List.map(c => Color.hex(c)) |> Array.of_list;
};

let get = (v: t, i, foreground, background) => {
  switch (i) {
  | 0 => foreground
  | 1 => background
  | c when i >= 0 && i < Array.length(v) => v[c]
  | _ => foreground
  };
};
