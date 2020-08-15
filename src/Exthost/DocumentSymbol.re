open Oni_Core;

[@deriving show]
type t = {
  name: string,
  detail: string,
  kind: SymbolKind.t,
  // TODO: tags
  containerName: option(string),
  range: OneBasedRange.t,
  selectionRange: OneBasedRange.t,
  children: list(t),
};

let decode =
  Json.Decode.fix(decoder => {
    Json.Decode.(
      obj(({field, _}) =>
        {
          name: field.required("name", string),
          detail: field.required("detail", string),
          kind: field.required("kind", SymbolKind.decode),
          containerName: field.optional("containerName", string),
          range: field.required("range", OneBasedRange.decode),
          selectionRange:
            field.required("selectionRange", OneBasedRange.decode),
          children: field.withDefault("children", [], list(decoder)),
        }
      )
    )
  });
