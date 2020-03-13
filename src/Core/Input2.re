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

	let equals = (_, _) => true;
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

type sequence = list(keyMatcher);

type binding = {
	id: int,
	sequence: sequence,
	payload: payload,
	enabled: context => bool,
};

type t = {
	nextId: int,
	allBindings: list(binding),
	keys: list(key),
};

let keyMatches = (keyMatcher, key) => {
	switch (keyMatcher) {
	| Scancode(scancode, mods) =>
	key.scancode == scancode && Modifiers.equals(mods, key.modifiers)
	| Keycode(keycode, mods) =>
	key.keycode == keycode && Modifiers.equals(mods, key.modifiers)
	}
};

let applyKeyToBinding = (key, binding) => {
	switch (binding.sequence) {
	| [hd, ...tail] when keyMatches(hd, key) => Some({
		...binding,
		sequence: tail
	})
	| [] => Some(binding)
	| _ => None
	}
};

let applyKeyToBindings = (key, bindings) => {
	List.filter_map(applyKeyToBinding(key), bindings);
};

let applyKeysToBindings = (keys, bindings) => {
	List.fold_left((acc, curr) => {
		applyKeyToBindings(curr, acc);
	}, bindings, keys);
};

let addBinding = (sequence, enabled, payload, bindings) => {
	let {nextId, allBindings, _} = bindings;
	let allBindings = [{
		id: nextId,
		sequence,
		payload,
		enabled,
	}, 
	...allBindings];

	let newBindings = {
		...bindings,
		allBindings,
		nextId: nextId + 1,
	};
	(newBindings, nextId);
};

let reset = (bindings) => {
	...bindings,
	keys: [],
};

let getReadyBindings = (bindings) => {
	let filter = (binding) => binding.sequence == [];

	bindings
	|> List.filter(filter);
};

let keyDown = (key, bindings) => {

	let keys = [key, ...bindings.keys];

	let candidateBindings =
	applyKeysToBindings(keys |> List.rev, bindings.allBindings);

	let readyBindings = getReadyBindings(candidateBindings);
	let candidateBindingCount = List.length(candidateBindings);

	switch (List.nth_opt(readyBindings, 0)) {
	| Some(binding) => 
		if (binding.sequence == [])  {
			(reset(bindings), [Execute(binding.payload)]);
		} else {
			({ ...bindings, keys }, []);
		};
	| None when candidateBindingCount > 0 => 
		({ ...bindings, keys }, []);
	| None => 
		(reset(bindings), [Unhandled(key)]);
	};
};

let keyUp = (_key, bindings) => (bindings, []);
let flush = (bindings) => (bindings, []);

let empty = {
	nextId: 0,
	allBindings: [],
	keys: [],
};
