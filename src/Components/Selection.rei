[@deriving show({with_path: false})]
type t =
  pri {
    anchor: int,
    focus: int,
  };

type directionVariant =
  | Left(int)
  | Right(int);

let initial: t;
let create: (~text: string, ~anchor: int, ~focus: int) => t;
let anchor: t => int;
let focus: t => int;
let range: t => int;
let rangeStart: t => int;
let rangeEnd: t => int;
let isCollapsed: t => bool;

let collapse: (~text: string, int) => t;
let collapseRelative: (~text: string, ~selection: t, directionVariant) => t;

let extend: (~text: string, ~selection: t, int) => t;
let extendRelative: (~text: string, ~selection: t, directionVariant) => t;
