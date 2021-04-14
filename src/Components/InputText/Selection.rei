[@deriving show({with_path: false})]
type t =
  pri {
    anchor: int,
    focus: int,
  };

let initial: t;

let create: (~text: string, ~anchor: int, ~focus: int) => t;
let all: string => t;
let length: t => int;
let offsetLeft: t => int;
let offsetRight: t => int;
let isCollapsed: t => bool;

let collapsed: (~text: string, int) => t;

let extend: (~text: string, ~selection: t, int) => t;
