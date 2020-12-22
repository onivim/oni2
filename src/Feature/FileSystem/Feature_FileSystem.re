open Oni_Core;

type model = {
	providers: list(unit)
};

let initial = {
	providers: []
};

type msg =
| Exthost(Exthost.Msg.FileSystem.msg, Lwt.u(Exthost.Reply.t));

module Msg = {
	let exthost = (~resolver, msg) => Exthost(msg, resolver);
}

type handle = int;

type outmsg =
| Nothing
| Effect(Isolinear.Effect.t(msg));

let update = (_msg, model) => (model, Nothing);

let getFileSystem = (~scheme as _, model) => None;

module Effects = {
	let readFile = (
		~handle as _,
		~uri as _,
		~toMsg as _,
		_model
	) => Isolinear.Effect.none;
}
