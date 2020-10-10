open Exthost;

[@deriving show]
type t = {
  chainedCacheId: option(ChainedCacheId.t),
  handle: int,
  label: string,
  kind: CompletionKind.t,
  detail: option(string),
  documentation: option(MarkdownString.t),
  insertText: string,
  insertTextRules: SuggestItem.InsertTextRules.t,
  sortText: string,
  suggestRange: option(SuggestItem.SuggestRange.t),
  commitCharacters: list(string),
  additionalTextEdits: list(Edit.SingleEditOperation.t),
  command: option(Command.t),
};

let create = (~handle, item: SuggestItem.t) => {
  chainedCacheId: item.chainedCacheId,
  handle,
  label: item.label,
  kind: item.kind,
  detail: item.detail,
  documentation: item.documentation,
  insertText: item |> SuggestItem.insertText,
  insertTextRules: item.insertTextRules,
  sortText: item |> SuggestItem.sortText,
  suggestRange: item.suggestRange,
  commitCharacters: item.commitCharacters,
  additionalTextEdits: item.additionalTextEdits,
  command: item.command,
};
