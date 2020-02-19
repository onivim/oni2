open Oni_Extensions;

module Msg {
	type t =
	| Resized(unit)
	| Updated(unit);
}

module Sub {
	let terminal: (~id: int, ~cmd: string, ~columns: int, ~rows: int, ~extClient: ExtHostClient.t) => Isolinear.Sub.t(Msg.t);
}

module Effect {
	let input: (~id: int, ~input: string) => Isolinear.Effect.t(Msg.t);
	let resize: (~id: int, ~rows: int, ~columns: int) => Isolinear.Effect.t(Msg.t);
}
