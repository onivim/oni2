[@deriving show({with_path: false})]
type t =
  pri {
    anchor: int,
    focus: int,
  };

let initial: t;
let anchor: t => int;
let focus: t => int;

let create: (~text: string, ~anchor: int, ~focus: int) => t;
let rangeWidth: t => int;
let rangeStart: t => int;
let rangeEnd: t => int;
let isCollapsed: t => bool;

let collapsed: (~text: string, int) => t;

let extend: (~text: string, ~selection: t, int) => t;
