open EditorCoreTypes;
open Oni_Core;
open Utility;
open Exthost;

[@deriving show]
type t = {
  chainedCacheId: option(ChainedCacheId.t),
  handle: option(int),
  label: string,
  kind: CompletionKind.t,
  detail: option(string),
  documentation: option(MarkdownString.t),
  insertText: string,
  insertTextRules: SuggestItem.InsertTextRules.t,
  filterText: string,
  sortText: string,
  suggestRange: SuggestItem.SuggestRange.t,
  commitCharacters: list(string),
  additionalTextEdits: list(Edit.SingleEditOperation.t),
  command: option(Command.t),
  score: float,
};

let create = (~handle, item: SuggestItem.t) => {
  chainedCacheId: item.chainedCacheId,
  handle: Some(handle),
  label: item.label,
  kind: item.kind,
  detail: item.detail,
  documentation: item.documentation,
  insertText: item |> SuggestItem.insertText,
  insertTextRules: item.insertTextRules,
  filterText: item |> SuggestItem.filterText,
  sortText: item |> SuggestItem.sortText,
  suggestRange: item.suggestRange,
  commitCharacters: item.commitCharacters,
  additionalTextEdits: item.additionalTextEdits,
  command: item.command,
  score: 0.,
};

let suggestRangeFromMeet = (~meet: CharacterPosition.t, ~base: string) => {
  let characters = Zed_utf8.length(base);
  Exthost.SuggestItem.SuggestRange.Single({
    startLineNumber: meet.line |> EditorCoreTypes.LineNumber.toOneBased,
    startColumn: (meet.character |> EditorCoreTypes.CharacterIndex.toInt) + 1,
    endLineNumber: meet.line |> EditorCoreTypes.LineNumber.toOneBased,
    endColumn:
      (meet.character |> EditorCoreTypes.CharacterIndex.toInt) + characters + 1,
  });
};

let keyword = (~meet: CharacterPosition.t, ~base, ~sortOrder: int, keyword) => {
  let sortText =
    "ZZZZ"
    ++ (string_of_int(sortOrder) |> StringEx.padFront(~totalLength=8, '0'));
  {
    chainedCacheId: None,
    handle: None,
    label: keyword,
    kind: Exthost.CompletionKind.Keyword,
    detail: None,
    documentation: None,
    insertText: keyword,
    insertTextRules: Exthost.SuggestItem.InsertTextRules.none,
    filterText: keyword,
    // Keywords should always be last, vs other completions...
    // But still sort them relative to each other
    sortText,
    suggestRange: suggestRangeFromMeet(~meet, ~base),
    commitCharacters: [],
    additionalTextEdits: [],
    command: None,
    score: 0.,
  };
};

let isKeyword = ({kind, _}) => kind == Exthost.CompletionKind.Keyword;

let snippet = (~meet, ~base, ~prefix: string, snippet: string) => {
  chainedCacheId: None,
  handle: None,
  label: prefix,
  kind: Exthost.CompletionKind.Snippet,
  detail: Some("Snippet"),
  documentation: Some(Exthost.MarkdownString.fromString(snippet)),
  insertText: snippet,
  insertTextRules: Exthost.SuggestItem.InsertTextRules.insertAsSnippet,
  filterText: prefix,
  sortText: prefix,
  suggestRange: suggestRangeFromMeet(~meet, ~base),
  commitCharacters: [],
  additionalTextEdits: [],
  command: None,
  score: 0.,
};

let replaceSpan = (~activeCursor: CharacterPosition.t, item: t) => {
  Exthost.SuggestItem.(
    switch (item.suggestRange) {
    | SuggestRange.Single({startColumn, endColumn, _}) =>
      let stop =
        max(endColumn - 1 |> CharacterIndex.ofInt, activeCursor.character);
      CharacterSpan.{start: startColumn - 1 |> CharacterIndex.ofInt, stop};
    | SuggestRange.Combo({insert, _}) =>
      let stop =
        max(
          insert.endColumn - 1 |> CharacterIndex.ofInt,
          activeCursor.character,
        );
      CharacterSpan.{
        start:
          Exthost.OneBasedRange.(
            insert.startColumn - 1 |> CharacterIndex.ofInt
          ),
        stop,
      };
    }
  );
};
