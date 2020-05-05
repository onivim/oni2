module Range = OneBasedRange;
open Oni_Core;

module Kind = {
	[@deriving show]
	type t =
	| Text
	| Read
	| Write;

	let ofInt = fun
	| 0 => Some(Text)
	| 1 => Some(Read)
	| 2 => Some(Write)
	| _ => None;

	let toInt = fun
	| Text => 0
	| Read => 1
	| Write => 2;

	let decode = Json.Decode.({
		int
		|> and_then(decoded => {
			switch (ofInt(decoded)) {
			| Some(v) => succeed(v)
			| None => fail("Invalid value for document highlight kind: " ++ string_of_int(decoded))
			}
		})
	});
};

[@deriving show]
type t = {
	range: OneBasedRange.t,
	kind: Kind.t,
}

let decode = Json.Decode.({
	obj(({field, _}) => {
		range: field.required("range", OneBasedRange.decode),
		kind: field.withDefault("kind", Kind.Text, Kind.decode),
	});
})
