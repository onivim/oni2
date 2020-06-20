open Oni_Core;

module ProviderMetadata = {
  [@deriving show]
  type t = {
    triggerCharacters: list(string),
    retriggerCharacters: list(string),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          triggerCharacters:
            field.withDefault("triggerCharacters", [], list(string)),
          retriggerCharacters:
            field.withDefault("retriggerCharacters", [], list(string)),
        }
      )
    );
};

module TriggerKind = {
  [@deriving show]
  type t =
    | Invoke
    | TriggerCharacter
    | ContentChange;

  let toInt =
    fun
    | Invoke => 1
    | TriggerCharacter => 2
    | ContentChange => 3;

  let ofInt =
    fun
    | 1 => Some(Invoke)
    | 2 => Some(TriggerCharacter)
    | 3 => Some(ContentChange)
    | _ => None;

  let encode = triggerKind => triggerKind |> toInt |> Json.Encode.int;
};

module RequestContext = {
  [@deriving show]
  type t = {
    triggerKind: TriggerKind.t,
    triggerCharacter: option(string),
    isRetrigger: bool,
    // TODO: Active signature help?
    //activate
  };

  let encode = ctx =>
    Json.Encode.(
      obj([
        ("triggerKind", ctx.triggerKind |> TriggerKind.encode),
        ("triggerCharacter", ctx.triggerCharacter |> nullable(string)),
        ("isRetrigger", ctx.isRetrigger |> bool),
      ])
    );
};
module ParameterInformation = {
  [@deriving show]
  type t = {
    label: [ | `String(string) | `Range(int, int)],
    documentation: option(MarkdownString.t),
  };

  let label =
    Json.Decode.(
      value
      |> and_then(
           fun
           | `String(_) as s => succeed(s)
           | `List(l) =>
             switch (l) {
             | [`Int(a), `Int(b)] => succeed(`Range((a, b)))
             | _ => fail("Expected a list with form [start, end]")
             }
           | _ => fail("Expected either a list or a string"),
         )
    );

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          label: field.required("label", label),
          documentation:
            field.optional("documentation", MarkdownString.decode),
        }
      )
    );
};

module Signature = {
  [@deriving show]
  type t = {
    label: string,
    documentation: option(MarkdownString.t),
    parameters: list(ParameterInformation.t),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          label: field.required("label", string),
          parameters:
            field.withDefault(
              "parameters",
              [],
              list(ParameterInformation.decode),
            ),
          documentation:
            field.optional("documentation", MarkdownString.decode),
        }
      )
    );
};

module Response = {
  [@deriving show]
  type t = {
    id: int,
    signatures: list(Signature.t),
    activeSignature: int,
    activeParameter: int,
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.required("id", int),
          signatures: field.required("signatures", list(Signature.decode)),
          activeSignature: field.required("activeSignature", int),
          activeParameter: field.required("activeParameter", int),
        }
      )
    );
};
