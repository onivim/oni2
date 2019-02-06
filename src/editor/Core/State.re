/*
 * State.re
 *
 * Top-level state of the editor
 */

open Types;

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
  buffer: Buffer.t,
};

let create: unit => t =
  () => {mode: Insert, buffer: Buffer.ofLines([||]), tabs: [Tab.create(0, "[No Name]")]};
