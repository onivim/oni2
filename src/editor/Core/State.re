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

module Tab = {
  type t = {
    id: int,
    title: string,
    active: bool,
  };

  let create = (id, title) => {id, title, active: false};
};

type t = {
  mode: Mode.t,
  tabs: list(Tab.t),
};

let create: unit => t =
  () => {mode: Insert, tabs: [Tab.create(0, "[No Name]")]};
