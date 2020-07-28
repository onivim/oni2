open EditorCoreTypes;

type model = {
  codeLens: CodeLens.model,
  definition: Definition.model,
  rename: Rename.model,
  references: References.model,
};

let initial = {
  codeLens: CodeLens.initial,
  definition: Definition.initial,
  rename: Rename.initial,
  references: References.initial,
};

[@deriving show]
type msg =
  | Exthost(Exthost.Msg.LanguageFeatures.msg)
  | Definition(Definition.msg)
  | References(References.msg)
  | Rename(Rename.msg)
  | CodeLens(CodeLens.msg)
  | KeyPressed(string)
  | Pasted(string);

type outmsg =
  | Nothing
  | OpenFile({
      filePath: string,
      location: option(Location.t),
    })
  | Effect(Isolinear.Effect.t(msg));

let map: ('a => msg, Outmsg.internalMsg('a)) => outmsg =
  f =>
    fun
    | Outmsg.Nothing => Nothing
    | Outmsg.OpenFile({filePath, location}) => OpenFile({filePath, location})
    | Outmsg.Effect(eff) => Effect(eff |> Isolinear.Effect.map(f));

module Msg = {
  let exthost = msg => Exthost(msg);

  let keyPressed = key => KeyPressed(key);
  let pasted = key => Pasted(key);
};

let update = (~maybeBuffer, ~cursorLocation, ~client, msg, model) =>
  switch (msg) {
  | KeyPressed(_)
  | Pasted(_) => (model, Nothing)
  | Exthost(RegisterCodeLensSupport({handle, selector, _})) =>
    let codeLens' = CodeLens.register(~handle, ~selector, model.codeLens);
    ({...model, codeLens: codeLens'}, Nothing);

  | Exthost(RegisterDefinitionSupport({handle, selector})) =>
    let definition' =
      Definition.register(~handle, ~selector, model.definition);
    ({...model, definition: definition'}, Nothing);

  | Exthost(RegisterReferenceSupport({handle, selector})) =>
    let references' =
      References.register(~handle, ~selector, model.references);
    ({...model, references: references'}, Nothing);

  | Exthost(Unregister({handle})) => (
      {
        codeLens: CodeLens.unregister(~handle, model.codeLens),
        definition: Definition.unregister(~handle, model.definition),
        references: References.unregister(~handle, model.references),
        rename: Rename.unregister(~handle, model.rename),
      },
      Nothing,
    )
  | Exthost(_) =>
    // TODO:
    (model, Nothing)

  | CodeLens(codeLensMsg) =>
    let codeLens' = CodeLens.update(codeLensMsg, model.codeLens);
    ({...model, codeLens: codeLens'}, Nothing);

  | Definition(definitionMsg) =>
    let (definition', outmsg) =
      Definition.update(definitionMsg, model.definition);
    (
      {...model, definition: definition'},
      outmsg |> map(msg => Definition(msg)),
    );

  | References(referencesMsg) =>
    let (references', outmsg) =
      References.update(
        ~maybeBuffer,
        ~cursorLocation,
        ~client,
        referencesMsg,
        model.references,
      );

    (
      {...model, references: references'},
      outmsg |> map(msg => References(msg)),
    );

  | Rename(renameMsg) =>
    let (rename', outmsg) = Rename.update(renameMsg, model.rename);
    ({...model, rename: rename'}, outmsg |> map(msg => Rename(msg)));
  };

let isFocused = ({rename, _}) => Rename.isFocused(rename);

module Contributions = {
  open WhenExpr.ContextKeys.Schema;

  let commands =
    (
      Rename.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => Rename(msg)))
    )
    @ (
      Definition.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => Definition(msg)))
    )
    @ (
      References.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => References(msg)))
    );

  let contextKeys =
    Rename.Contributions.contextKeys
    |> fromList
    |> map(({rename, _}: model) => rename);

  let keybindings =
    Rename.Contributions.keybindings @ Definition.Contributions.keybindings;
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
