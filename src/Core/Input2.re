// TODO: Functorize
type payload = string;
type context = bool;

module Modifiers = {
	type t = {
		control: bool,
		alt: bool,
		shift: bool, 
		meta: bool,
	};

	let create = (~control, ~alt, ~shift, ~meta) => {
		control,
		alt,
		shift,
		meta,
	};

	let none = {
		control: false,
		alt: false,
		shift: false,
		meta: false,
	};
};

type key = {
	scancode: int,
	keycode: int,
	modifiers: Modifiers.t,
	text: string,
};


type effects =
| Execute(payload)
| Unhandled(key);

type keyMatcher = 
| Scancode(int, Modifiers.t)
| Keycode(int, Modifiers.t);

type chord = list(keyMatcher);

type sequence = list(chord);

type binding = {
	id: int,
	matcher: sequence,
	payload: payload,
	enabled: context => bool,
};

type t = {
	nextId: int,
	allBindings: list(binding),
};

let addBinding = (matcher, enabled, payload, bindings) => {
	let {nextId, allBindings} = bindings;
	let allBindings = [{
		id: nextId,
		matcher,
		payload,
		enabled,
	}, 
	...allBindings];

	let newBindings = {
		allBindings,
		nextId: nextId + 1,
	};
	(newBindings, nextId);
};

let keyDown = (key, bindings) => (bindings, []);
let keyUp = (key, bindings) => (bindings, []);
let flush = (bindings) => (bindings, []);

let empty = {
	nextId: 0,
	allBindings: [],
};
