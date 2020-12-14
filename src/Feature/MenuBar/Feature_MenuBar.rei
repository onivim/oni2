
open Oni_Core;

type model;

let initial: model;

module View: {
	let make: (
		~theme: ColorTheme.Colors.t,
		~config: Config.resolver,
		~model: model,
		unit
	) => Revery.UI.element;
};
