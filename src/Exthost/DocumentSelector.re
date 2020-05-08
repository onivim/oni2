open Oni_Core;

[@deriving show]
type t = list(DocumentFilter.t);

let decode = Json.Decode.(
	list(DocumentFilter.decode)
);

let matches = (~filetype: string, filter) => {
	filter
	|> List.exists(DocumentFilter.matches(~filetype));
};
