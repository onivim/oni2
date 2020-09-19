open EditorCoreTypes;
open Oni_Core;

type model;

let initial: model;

[@deriving show]
type msg;

module Msg: {
  let exthost: Exthost.Msg.LanguageFeatures.msg => msg;
  let keyPressed: string => msg;
  let pasted: string => msg;

  module Formatting: {
    let formatDocument: msg;
    let formatRange:
      (
        ~startLine: EditorCoreTypes.LineNumber.t,
        ~endLine: EditorCoreTypes.LineNumber.t
      ) =>
      msg;
  };

  module Hover: {
    let show: msg;
    let mouseHovered: CharacterPosition.t => msg;
    let mouseMoved: CharacterPosition.t => msg;
    let keyPressed: string => msg;
  };
};

type outmsg =
  | Nothing
  | ApplyCompletion({
      meetColumn: CharacterIndex.t,
      insertText: string,
    })
  | InsertSnippet({
      meetColumn: CharacterIndex.t,
      snippet: string,
    })
  | OpenFile({
      filePath: string,
      location: option(CharacterPosition.t),
    })
  | NotifySuccess(string)
  | NotifyFailure(string)
  | Effect(Isolinear.Effect.t(msg));

let update:
  (
    ~configuration: Oni_Core.Configuration.t,
    ~languageConfiguration: Oni_Core.LanguageConfiguration.t,
    ~maybeSelection: option(CharacterRange.t),
    ~maybeBuffer: option(Oni_Core.Buffer.t),
    ~editorId: int,
    ~cursorLocation: CharacterPosition.t,
    ~client: Exthost.Client.t,
    msg,
    model
  ) =>
  (model, outmsg);

let bufferUpdated:
  (
    ~buffer: Oni_Core.Buffer.t,
    ~config: Oni_Core.Config.resolver,
    ~activeCursor: CharacterPosition.t,
    ~syntaxScope: Oni_Core.SyntaxScope.t,
    ~triggerKey: option(string),
    model
  ) =>
  model;
let cursorMoved:
  (~previous: CharacterPosition.t, ~current: CharacterPosition.t, model) =>
  model;
let startInsertMode: model => model;
let stopInsertMode: model => model;
let isFocused: model => bool;

let sub:
  (
    ~isInsertMode: bool,
    ~activeBuffer: Oni_Core.Buffer.t,
    ~activePosition: CharacterPosition.t,
    ~visibleBuffers: list(Oni_Core.Buffer.t),
    ~client: Exthost.Client.t,
    model
  ) =>
  Isolinear.Sub.t(msg);

module Completion: {
  let isActive: model => bool;

  let providerCount: model => int;

  let availableCompletionCount: model => int;

  module View: {
    let make:
      (
        ~x: int,
        ~y: int,
        ~lineHeight: float,
        ~theme: Oni_Core.ColorTheme.Colors.t,
        ~tokenTheme: Oni_Syntax.TokenTheme.t,
        ~editorFont: Service_Font.font,
        ~model: model,
        unit
      ) =>
      Revery.UI.element;
  };
};

module Hover: {
  module Popup: {
    let make:
      (
        ~diagnostics: Feature_Diagnostics.model,
        ~theme: Oni_Core.ColorTheme.Colors.t,
        ~tokenTheme: Oni_Syntax.TokenTheme.t,
        ~languageInfo: Exthost.LanguageInfo.t,
        ~uiFont: UiFont.t,
        ~editorFont: Service_Font.font,
        ~grammars: Oni_Syntax.GrammarRepository.t,
        ~model: model,
        ~buffer: Oni_Core.Buffer.t,
        ~editorId: option(int)
      ) =>
      option((CharacterPosition.t, list(Oni_Components.Popup.Section.t)));
  };
};

module Contributions: {
  let colors: list(ColorTheme.Schema.definition);
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
  let contextKeys: WhenExpr.ContextKeys.Schema.t(model);
  let keybindings: list(Oni_Input.Keybindings.keybinding);
};

module Definition: {
  let get: (~bufferId: int, model) => option(Exthost.DefinitionLink.t);

  let getAt:
    (~bufferId: int, ~range: CharacterRange.t, model) =>
    option(Exthost.DefinitionLink.t);

  let isAvailable: (~bufferId: int, model) => bool;
};

module DocumentHighlights: {
  let getByLine:
    (~bufferId: int, ~line: int, model) => list(CharacterRange.t);

  let getLinesWithHighlight: (~bufferId: int, model) => list(int);
};

// TODO: Remove
module CompletionMeet = CompletionMeet;
module LanguageFeatures = LanguageFeatures;
