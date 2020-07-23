open Common;
type model = {
  codeLens: CodeLens.model,
  rename: Rename.model,
};

let initial = {rename: Rename.initial, codeLens: CodeLens.initial};

[@deriving show]
type msg =
  | Exthost(Exthost.Msg.LanguageFeatures.msg)
  | Rename(Rename.msg)
  | CodeLens(CodeLens.msg)
  | KeyPressed(string)
  | Pasted(string);

module Msg = {
  let exthost = msg => Exthost(msg);

  let keyPressed = key => KeyPressed(key);
  let pasted = key => Pasted(key);
};

type outmsg = Common.Outmsg.t;

let update = (msg, model) =>
  switch (msg) {
  | KeyPressed(_)
  | Pasted(_) => (model, Outmsg.Nothing)
  | Exthost(RegisterCodeLensSupport({handle, selector, _})) =>
    let codeLens' = CodeLens.register(~handle, ~selector, model.codeLens);
    ({...model, codeLens: codeLens'}, Outmsg.Nothing);
  | Exthost(Unregister({handle})) => (
      {
        codeLens: CodeLens.unregister(~handle, model.codeLens),
        rename: Rename.unregister(~handle, model.rename),
      },
      Outmsg.Nothing,
    )
  | Exthost(_) =>
    // TODO:
    (model, Outmsg.Nothing)
  | CodeLens(codeLensMsg) =>
    let codeLens' = CodeLens.update(codeLensMsg, model.codeLens);
    ({...model, codeLens: codeLens'}, Outmsg.Nothing);
  | Rename(renameMsg) =>
    let (rename', outmsg) = Rename.update(renameMsg, model.rename);
    ({...model, rename: rename'}, outmsg);
  };

let isFocused = ({rename, _}) => Rename.isFocused(rename);

module Contributions = {
  open WhenExpr.ContextKeys.Schema;

  let commands =
    Rename.Contributions.commands
    |> List.map(Oni_Core.Command.map(msg => Rename(msg)));

  let contextKeys =
    Rename.Contributions.contextKeys
    |> fromList
    |> map(({rename, _}: model) => rename);

  let keybindings = Rename.Contributions.keybindings;
};

let sub = (~visibleBuffers, ~client) => {
  CodeLens.sub(~visibleBuffers, ~client)
  |> Isolinear.Sub.map(msg => CodeLens(msg));
};

// TODO: Remove
module Completions = Completions;
module CompletionItem = CompletionItem;
module CompletionMeet = CompletionMeet;
module Definition = Definition;
module Diagnostic = Diagnostic;
module Diagnostics = Diagnostics;
module LanguageFeatures = LanguageFeatures;
