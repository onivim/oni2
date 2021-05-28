[@deriving show]
type t;

let parse: string => result(t, string);

let matches: (t, string) => bool;

let toDebugString: t => string;

let decode: Json.decoder(t);
