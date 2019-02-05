/*
 * State.re
 *
 * Top-level state of the editor
 */

module Mode = {
  type t =
    | Insert
    | Normal
    | Other;

  let show = v =>
    switch (v) {
    | Insert => "insert"
    | Normal => "normal"
    | Other => "unknown"
    };
};

type t = {mode: Mode.t};

let create: unit => t = () => {mode: Insert};
