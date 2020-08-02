module ExtCommand = Command;
open Oni_Core;

module SuggestRange = {
  [@deriving show]
  type t =
    | Single(OneBasedRange.t)
    | Combo({
        insert: OneBasedRange.t,
        replace: OneBasedRange.t,
      });

  module Decode = {
    open Json.Decode;
    let single = OneBasedRange.decode |> map(range => Single(range));

    let combo =
      obj(({field, _}) =>
        Combo({
          insert: field.required("insert", OneBasedRange.decode),
          replace: field.required("replace", OneBasedRange.decode),
        })
      );
  };

  let decode =
    Json.Decode.one_of([
      ("single", Decode.single),
      ("combo", Decode.combo),
    ]);

  let encode = suggestRange =>
    switch (suggestRange) {
    | Single(range) => OneBasedRange.encode(range)
    | Combo({insert, replace}) =>
      Json.Encode.(
        {
          obj([
            ("insert", insert |> OneBasedRange.encode),
            ("replace", replace |> OneBasedRange.encode),
          ]);
        }
      )
    };
};

[@deriving show]
type t = {
  chainedCacheId: option(ChainedCacheId.t),
  label: string,
  kind: CompletionKind.t,
  detail: option(string),
  documentation: option(MarkdownString.t),
  sortText: option(string),
  filterText: option(string),
  insertText: option(string),
  suggestRange: option(SuggestRange.t),
  commitCharacters: list(string),
  additionalTextEdits: list(Edit.SingleEditOperation.t),
  command: option(ExtCommand.t),
  // TODO:
  // insertTextRules
  // kindModifer
  // chainedCacheId?
};

let insertText = ({label, insertText, _}) => {
  // TODO: Consider the `insertText` value
  // There are some cases we don't handle - for example, the JS / TS provider
  // prefixes the insertText with a `.` - we'd need to examine how we are
  // applying completion in that context.
  switch (insertText) {
  | Some(insert) => insert
  | None => label
  };
};

let filterText = ({filterText, label, _}) => {
  switch (filterText) {
  | None => label
  | Some(filter) => filter
  };
};

let sortText = ({sortText, label, _}) => {
  switch (sortText) {
  | None => label
  | Some(sortText) => sortText
  };
};

module Dto = {
  let decode = {
    Json.Decode.(
      obj(({field, _}) => {
        // These fields come from the `ISuggestDataDtoField` definition:
        // https://github.com/onivim/vscode-exthost/blob/50bef147f7bbd250015361a4e3cad3305f65bc27/src/vs/workbench/api/common/extHost.protocol.ts#L1089
        let label = field.required("a", string);

        let kind =
          field.required("b", int)
          |> CompletionKind.ofInt
          |> Option.value(~default=CompletionKind.Method);

        let detail = field.optional("c", string);
        let documentation = field.optional("d", MarkdownString.decode);
        let sortText = field.optional("e", string);
        let filterText = field.optional("f", string);
        let insertText = field.optional("h", string);
        let suggestRange = field.optional("j", SuggestRange.decode);
        let commitCharacters = field.withDefault("k", [], list(string));
        let additionalTextEdits =
          field.withDefault("l", [], list(Edit.SingleEditOperation.decode));
        let command = field.optional("m", ExtCommand.decode);
        let chainedCacheId = field.optional("x", ChainedCacheId.decode);
        {
          chainedCacheId,
          label,
          kind,
          detail,
          documentation,
          sortText,
          filterText,
          insertText,
          suggestRange,
          commitCharacters,
          additionalTextEdits,
          command,
        };
      })
    );
  };
};

let encode = suggestItem =>
  Json.Encode.(
    {
      let suggestRange =
        suggestItem.suggestRange
        |> Option.value(~default=SuggestRange.Single(OneBasedRange.one));
      obj([
        ("label", string(suggestItem.label)),
        ("kind", suggestItem.kind |> CompletionKind.toInt |> int),
        ("range", suggestRange |> SuggestRange.encode),
        ("insertText", suggestItem |> insertText |> string),
      ]);
    }
  );
