open Oni_Core;

module ProviderMetadata = {
    [@deriving show]
	type t = {
		triggerCharacters: list(string),
		retriggerCharacters: list(string),
	}

	let decode = Json.Decode.(obj(({field, _}) => {
		triggerCharacters: field.withDefault("triggerCharacters", [], list(string)),
		retriggerCharacters: field.withDefault("retriggerCharacters", [], list(string)),
	}))
};
