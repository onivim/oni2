
type t;

let enabled: t;

let disabled: t;

let ofFeatures: list(string) => t;

let decode: Json.Decode.decoder(t);

let toHarfbuzzFeatures: t => list(Harfbuzz.feature);
