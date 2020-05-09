open Oni_Core;

type t = string;

let decode =
  Json.Decode.(
    {
      let simple = string;

      let value = obj(({field, _}) => {field.required("value", string)});

      one_of([("simple", simple), ("value", value)]);
    }
  );
