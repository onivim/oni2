type model = unit;

[@deriving show]
type msg = unit;


let update = (model, _msg) => model;

module Sub = {
	let create = (~buffers, ~client) => {
		ignore(buffers);
		ignore(client);
		Isolinear.Sub.none;
	};
};

let sub = Sub.create;
