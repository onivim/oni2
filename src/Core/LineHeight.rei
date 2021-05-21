[@deriving show]
type t;

let absolute: float => t;

let proportional: float => t;

let padding: int => t;

let default: t;

let calculate: (~measuredFontHeight: float, t) => float;

let decode: Json.decoder(t);
let encode: Json.encoder(t);
