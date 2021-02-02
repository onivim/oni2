type t;

let enabled: t;

let disabled: t;

let ofFeatures: list(string) => t;

let toHarfbuzzFeatures: t => list(Harfbuzz.feature);

let decode: Json.Decode.decoder(t);
let encode: Json.Encode.encoder(t);
