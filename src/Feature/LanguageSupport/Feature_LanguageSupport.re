type model = {rename: Rename.model};

let initial = {rename: Rename.initial};

[@deriving show]
type msg =
  | Exthost(Exthost.Msg.LanguageFeatures.msg)
  | Rename(Rename.msg);

module Msg = {
  let exthost = msg => Exthost(msg);
};

type outmsg = Common.Outmsg.t;

let update = (msg, model) =>
  switch (msg) {
  | Exthost(_) =>
    // TODO:
    (model, ())
  | Rename(renameMsg) =>
    let (rename', outmsg) = Rename.update(renameMsg, model.rename);
    ({rename: rename'}, outmsg);
  };

module Contributions = {
  open WhenExpr.ContextKeys.Schema;

  let commands =
    Rename.Contributions.commands
    |> List.map(Oni_Core.Command.map(msg => Rename(msg)));

  let contextKeys =
    Rename.Contributions.contextKeys
    |> fromList
    |> map(({rename}: model) => rename);

  let keybindings = Rename.Contributions.keybindings;
};

// TODO: Remove
module Completions = Completions;
module CompletionItem = CompletionItem;
module CompletionMeet = CompletionMeet;
module Definition = Definition;
module Diagnostic = Diagnostic;
module Diagnostics = Diagnostics;
module LanguageFeatures = LanguageFeatures;
