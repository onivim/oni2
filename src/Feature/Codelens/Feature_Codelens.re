open Oni_Core;

type lenses = {
	lenses: list(Exthost.CodeLens.t);
};

type model = {
	bufferToLenses: IntMap.t(lenses);
};

[@deriving show]
type msg = 
| CodelensProviderAvailable({
	handle: int,
	selector: Exthost.DocumentSelector.t,
})
| CodelensesReceived({
	bufferId: int, 
	lenses:list(Exthost.CodeLens.t)
	});


let update = (model, _msg) => model;

module Sub = {
	let create = (~buffers, ~client) => {
		ignore(buffers);
		ignore(client);
		Isolinear.Sub.none;
	};
};

let sub = Sub.create;
