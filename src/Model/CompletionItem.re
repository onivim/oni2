module Ext = Oni_Extensions;

type t = {
  label: string,
  kind: option(Ext.CompletionItemKind.t),
  detail: option(string),
};

let create = (~kind, ~detail, label) => {label, kind, detail};

let toString = completionItem =>
  Printf.sprintf("CompletionItem - Label: %s", completionItem.label);
