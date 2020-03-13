type payload = string;
type context = bool;

type t;

module Modifiers: {
	type t;

	let create: (~control: bool,
	~alt: bool, 
	~shift: bool,
	~meta: bool) => t;

	let none: t;

	let equals: (t, t) => bool;
};

type key = {
	scancode: int,
	keycode: int,
	modifiers: Modifiers.t,
	text: string,
};

type keyMatcher = 
| Scancode(int, Modifiers.t)
| Keycode(int, Modifiers.t);

type sequence = list(keyMatcher);

let addBinding: (sequence, context => bool, payload, t) =>
(t, int);
//let removeBinding: (t, int) => t;

type effects =
| Execute(payload)
| Unhandled(key);

let keyDown: (key, t) => (t, list(effects));
let keyUp: (key, t) => (t, list(effects));
let flush: t => (t, list(effects));

let empty: t;
