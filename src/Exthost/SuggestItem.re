module ExtCommand = Command;
open Oni_Core;

module InsertTextRules = {
  // Must be kept in sync:
  // https://github.com/onivim/vscode-exthost/blob/c7df89c1cf0087ca5decaf8f6d4c0fd0257a8b7a/src/vs/editor/common/modes.ts#L439 //
  [@deriving show]
  type rule =
    | KeepWhitespace // 0b001
    | InsertAsSnippet; // 0b100

  [@deriving show]
  type t = list(rule);

  let none = [];

  let insertAsSnippet = [InsertAsSnippet];

  let ofInt = ruleAsInt => {
    let keepWhitespace = ruleAsInt land 0b001 == 0b001;
    let insertAsSnippet = ruleAsInt land 0b100 == 0b100;

    switch (keepWhitespace, insertAsSnippet) {
    | (true, true) => [KeepWhitespace, InsertAsSnippet]
    | (true, false) => [KeepWhitespace]
    | (false, true) => [InsertAsSnippet]
    | (false, false) => []
    };
  };

  let matches = (~rule: rule, rules) => {
    rules |> List.exists(r => r == rule);
  };

  let decode =
    Json.Decode.(
      {
        int |> map(ofInt);
      }
    );
};

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
  insertTextRules: InsertTextRules.t,
  suggestRange: SuggestRange.t,
  commitCharacters: list(string),
  additionalTextEdits: list(Edit.SingleEditOperation.t),
  command: option(ExtCommand.t),
  // TODO:
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

module SuggestItemLabel = {
  type t = string;

  let decode =
    Json.Decode.(
      one_of([
        ("string", string),
        ("record", obj(({field, _}) => {field.required("label", string)})),
      ])
    );
};

module Dto = {
  let decode = (~defaultRange) => {
    Json.Decode.(
      obj(({field, _}) => {
        // These fields come from the `ISuggestDataDtoField` definition:
        // https://github.com/onivim/vscode-exthost/blob/50bef147f7bbd250015361a4e3cad3305f65bc27/src/vs/workbench/api/common/extHost.protocol.ts#L1089
        let label = field.required("a", SuggestItemLabel.decode);

        let kind =
          field.withDefault(
            "b",
            CompletionKind.Property,
            int
            |> map(i =>
                 CompletionKind.ofInt(i)
                 |> Option.value(~default=CompletionKind.Property)
               ),
          );

        let detail = field.withDefault("c", None, nullable(string));
        let documentation =
          field.withDefault("d", None, nullable(MarkdownString.decode));
        let sortText = field.withDefault("e", None, nullable(string));
        let filterText = field.withDefault("f", None, nullable(string));
        let insertText = field.withDefault("h", None, nullable(string));
        let insertTextRules =
          field.withDefault(
            "i",
            InsertTextRules.none,
            InsertTextRules.decode,
          );
        let suggestRange =
          field.optional("j", SuggestRange.decode)
          |> Option.value(~default=defaultRange);
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
          insertTextRules,
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
      obj([
        ("label", string(suggestItem.label)),
        ("kind", suggestItem.kind |> CompletionKind.toInt |> int),
        ("range", suggestItem.suggestRange |> SuggestRange.encode),
        ("insertText", suggestItem |> insertText |> string),
      ]);
    }
  );
