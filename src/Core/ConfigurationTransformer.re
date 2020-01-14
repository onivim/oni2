/*
 * ConfigurationTransformer.re
 *
 * Helpers for implementing configuration 'transformers' -
 * functions that update configurations.
 */
module Log = (val Log.withNamespace("Oni2.Core.ConfigurationTransformer"));

type t = Yojson.Safe.t => Yojson.Safe.t;

let setField = (fieldName, value, json) => {
  switch (json) {
  | `Assoc(elems) =>
    let filtered =
      elems |> List.filter(((key, _)) => !String.equal(key, fieldName));

    let newItems = [(fieldName, value), ...filtered];
    `Assoc(newItems);
  | _ =>
    Log.warn("Unable to transform json - not an association list");
    json;
  };
};
