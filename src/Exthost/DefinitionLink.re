open Oni_Core;
type t = {
  uri: Uri.t,
  range: OneBasedRange.t,
  originSelectionRange: option(OneBasedRange.t),
  targetSelectionRange: option(OneBasedRange.t),
};

let decode = {
  Json.Decode.(
    obj(({field, _}) =>
      {
        uri: field.required("uri", Uri.decode),
        range: field.required("range", OneBasedRange.decode),
        originSelectionRange: field.optional("originSelectionRange", OneBasedRange.decode),
        targetSelectionRange: field.optional("targetSelectionRange", OneBasedRange.decode),
      }
    )
  );
};
