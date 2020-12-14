module Schema: {
	type item;
	type menu;

	let item: (
		~title: string,
		~command: string,
		~parent: menu,
	) => item;

	let menu: (
		~title: string,
		~parent: option(menu),
	) => menu;

	type t;

	let toSchema: (~items: list(item), ~menus: list(menu)) => t;
	let union: (t, t) => t;
}

type t;

let initial: Schema.t => t;

type builtMenu('msg);

let build: (~commands: Command.Lookup.t('msg), t) => builtMenu('msg);
