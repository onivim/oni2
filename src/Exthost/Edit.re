open Oni_Core;

module SingleEditOperation = {
  type t = {
    range: OneBasedRange.t,
    text: option(string),
    forceMoveMarkers: bool,
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          range: field.required("range", OneBasedRange.decode),
          text: field.optional("text", string),
          forceMoveMarkers:
            field.withDefault("forceMoveMarkers", false, bool),
        }
      )
    );
};
