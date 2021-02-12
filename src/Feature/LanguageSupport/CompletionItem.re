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
  suggestRange: option(SuggestItem.SuggestRange.t),
  commitCharacters: list(string),
  additionalTextEdits: list(Edit.SingleEditOperation.t),
  command: option(Command.t),
  isFuzzyMatching: bool,
};

let create = (~isFuzzyMatching: bool, ~handle, item: SuggestItem.t) => {
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
  isFuzzyMatching,
};

let keyword = (~sortOrder: int, ~isFuzzyMatching, keyword) => {
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
    suggestRange: None,
    commitCharacters: [],
    additionalTextEdits: [],
    command: None,
    isFuzzyMatching,
  };
};

let isKeyword = ({kind, _}) => kind == Exthost.CompletionKind.Keyword;

let snippet = (~isFuzzyMatching, ~prefix: string, snippet: string) => {
  chainedCacheId: None,
  handle: None,
  label: prefix,
  kind: Exthost.CompletionKind.Snippet,
  detail: Some(snippet),
  documentation: None,
  insertText: snippet,
  insertTextRules: Exthost.SuggestItem.InsertTextRules.insertAsSnippet,
  filterText: prefix,
  sortText: prefix,
  suggestRange: None,
  commitCharacters: [],
  additionalTextEdits: [],
  command: None,
  isFuzzyMatching,
};
