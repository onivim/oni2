open Common;
type model = {
  codeLens: CodeLens.model,
  definition: Definition.model,
  rename: Rename.model,
};

let initial = {
  codeLens: CodeLens.initial,
  definition: Definition.initial,
  rename: Rename.initial,
};

[@deriving show]
type msg =
  | Exthost(Exthost.Msg.LanguageFeatures.msg)
  | Definition(Definition.msg)
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

  | Exthost(RegisterDefinitionSupport({handle, selector})) =>
    let definition' =
      Definition.register(~handle, ~selector, model.definition);
    ({...model, definition: definition'}, Outmsg.Nothing);

  | Exthost(Unregister({handle})) => (
      {
        codeLens: CodeLens.unregister(~handle, model.codeLens),
        definition: Definition.unregister(~handle, model.definition),
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

  | Definition(definitionMsg) =>
    let definition' = Definition.update(definitionMsg, model.definition);
    ({...model, definition: definition'}, Outmsg.Nothing);

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

module OldDefinition = Definition;
module Definition = {
  let get = (~bufferId, {definition, _}: model) => {
    OldDefinition.get(~bufferId, definition);
  };

  let getAt = (~bufferId, ~range, {definition, _}: model) => {
    OldDefinition.getAt(~bufferId, ~range, definition);
  };

  let isAvailable = (~bufferId, model) => {
    model |> get(~bufferId) |> Option.is_some;
  };
};

let sub =
    (
      ~isInsertMode,
      ~activeBuffer,
      ~activePosition,
      ~visibleBuffers,
      ~client,
      {definition, _},
    ) => {
  let codeLensSub =
    CodeLens.sub(~visibleBuffers, ~client)
    |> Isolinear.Sub.map(msg => CodeLens(msg));

  let definitionSub =
    isInsertMode
      ? Isolinear.Sub.none
      : OldDefinition.sub(
          ~buffer=activeBuffer,
          ~location=activePosition,
          ~client,
          definition,
        )
        |> Isolinear.Sub.map(msg => Definition(msg));

  [codeLensSub, definitionSub] |> Isolinear.Sub.batch;
};

// TODO: Remove
module Completions = Completions;
module CompletionItem = CompletionItem;
module CompletionMeet = CompletionMeet;
module Diagnostic = Diagnostic;
module Diagnostics = Diagnostics;
module LanguageFeatures = LanguageFeatures;
