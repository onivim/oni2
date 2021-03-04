type t =
  | Enabled
  | Disabled
  | Features(list(string));

let enabled = Enabled;

let disabled = Disabled;

let ofFeatures = features => Features(features);

let toHarfbuzzFeatures =
  fun
  // If ligatures is [true], just enable all features
  | Enabled => []

  | Disabled =>
    Revery.Font.[
      Feature.make(~tag=Features.contextualAlternates, ~value=0),
      Feature.make(~tag=Features.standardLigatures, ~value=0),
    ]

  | Features(features) =>
    features
    |> List.map(tag => {
         //Log.infof(m => m("Enabling font feature: %s", tag));
         let tag = Revery_Font.Feature.customTag(tag);
         Revery.Font.Feature.make(~tag, ~value=1);
       });

let decode = {
  open Json.Decode;
  let decodeBool =
    bool
    |> map(
         fun
         | true => enabled
         | false => disabled,
       );

  let decodeString =
    string
    |> map(str => {
         open Angstrom;

         let quoted = p => char('\'') *> p <* char('\'');

         let isAlphaNumeric =
           fun
           | 'a' .. 'z'
           | 'A' .. 'Z'
           | '0' .. '9' => true
           | _ => false;

         let alphaString = take_while1(isAlphaNumeric);

         let feature = quoted(alphaString);
         let spaces = many(char(' '));

         let parse = sep_by(char(',') <* spaces, feature);

         switch (Angstrom.parse_string(~consume=All, parse, str)) {
         | Ok(list) => ofFeatures(list)
         | Error(_msg) => enabled
         };
       });

  one_of([
    ("FontLigatures.bool", decodeBool),
    ("FontLigatures.string", decodeString),
  ]);
};

let encode =
  Json.Encode.(
    fun
    | Enabled => bool(true)
    | Disabled => bool(false)
    | Features(features) =>
      string(
        features |> List.map(feat => "'" ++ feat ++ "'") |> String.concat(","),
      )
  );
