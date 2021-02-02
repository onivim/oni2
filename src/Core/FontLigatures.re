
type t =
| Enabled
| Disabled
| Features(list(string));

let enabled = Enabled;

let disabled = Disabled;

let ofFeatures = features => Features(features);

let toHarfbuzzFeatures = fun
// If ligatures is [true], just enable all features
| Enabled=> []

| Disabled => Revery.Font.[
            Feature.make(
              ~tag=Features.contextualAlternates,
              ~value=0,
            ),
            Feature.make(
              ~tag=Features.standardLigatures,
              ~value=0,
            ),
]

| Features(features) => features
          |> List.map(tag => {
               //Log.infof(m => m("Enabling font feature: %s", tag));
               let tag = Revery_Font.Feature.customTag(tag);
               Revery.Font.Feature.make(~tag, ~value=1);
             });

let decode = {
    open Json.Decode;

    succeed(Enabled);
};
