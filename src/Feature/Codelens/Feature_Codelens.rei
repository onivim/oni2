type model;

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

let update: (model, msg) => model;

let sub: (
	~buffers: list(Oni_Core.Buffer.t),
	~client: Exthost.Client.t,
) => Isolinear.Sub.t(msg);

