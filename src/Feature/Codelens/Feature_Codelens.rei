type model;

[@deriving show]
type msg;

let update: (model, msg) => model;

let sub: (
	~buffers: list(Oni_Core.Buffer.t),
	~client: Exthost.Client.t,
) => Isolinear.Sub.t(msg);

