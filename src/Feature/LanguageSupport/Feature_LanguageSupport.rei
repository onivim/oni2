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
    let mouseHovered: option(CharacterPosition.t) => msg;
    let mouseMoved: option(CharacterPosition.t) => msg;
    let keyPressed: string => msg;
  };
};

module CodeLens: {
  type t = Exthost.CodeLens.lens;

  let lineNumber: t => int;
  let text: t => string;

  module View: {
    let make:
      (
        ~leftMargin: int,
        ~theme: Oni_Core.ColorTheme.Colors.t,
        ~uiFont: UiFont.t,
        ~codeLens: t,
        unit
      ) =>
      Revery.UI.element;
  };
};

type outmsg =
  | Nothing
  | ApplyCompletion({
      meetColumn: CharacterIndex.t,
      insertText: string,
      additionalEdits: list(Exthost.Edit.SingleEditOperation.t),
    })
  | InsertSnippet({
      meetColumn: CharacterIndex.t,
      snippet: string,
      additionalEdits: list(Exthost.Edit.SingleEditOperation.t),
    })
  | OpenFile({
      filePath: string,
      location: option(CharacterPosition.t),
    })
  | ReferencesAvailable
  | NotifySuccess(string)
  | NotifyFailure(string)
  | Effect(Isolinear.Effect.t(msg))
  | CodeLensesChanged({
      handle: int,
      bufferId: int,
      startLine: EditorCoreTypes.LineNumber.t,
      stopLine: EditorCoreTypes.LineNumber.t,
      lenses: list(CodeLens.t),
    })
  | SetSelections({
      editorId: int,
      ranges: list(CharacterRange.t),
    })
  | ShowMenu(Feature_Quickmenu.Schema.menu(msg))
  | TransformConfiguration(ConfigurationTransformer.t);

let update:
  (
    ~config: Oni_Core.Config.resolver,
    ~extensions: Feature_Extensions.model,
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
    ~languageConfiguration: Oni_Core.LanguageConfiguration.t,
    ~buffer: Oni_Core.Buffer.t,
    ~config: Oni_Core.Config.resolver,
    ~extensions: Feature_Extensions.model,
    ~activeCursor: CharacterPosition.t,
    ~syntaxScope: Oni_Core.SyntaxScope.t,
    ~triggerKey: option(string),
    model
  ) =>
  model;

let configurationChanged: (~config: Config.resolver, model) => model;

let cursorMoved:
  (
    ~languageConfiguration: Oni_Core.LanguageConfiguration.t,
    ~buffer: Oni_Core.Buffer.t,
    ~previous: CharacterPosition.t,
    ~current: CharacterPosition.t,
    model
  ) =>
  model;

let moveMarkers:
  (~newBuffer: Buffer.t, ~markerUpdate: MarkerUpdate.t, model) => model;

let startInsertMode:
  (
    ~config: Oni_Core.Config.resolver,
    ~maybeBuffer: option(Oni_Core.Buffer.t),
    model
  ) =>
  model;

let stopInsertMode: model => model;

let startSnippet: model => model;
let stopSnippet: model => model;

let isFocused: model => bool;

let sub:
  (
    ~config: Oni_Core.Config.resolver,
    ~isInsertMode: bool,
    ~isAnimatingScroll: bool,
    ~activeBuffer: Oni_Core.Buffer.t,
    ~activePosition: CharacterPosition.t,
    ~topVisibleBufferLine: EditorCoreTypes.LineNumber.t,
    ~bottomVisibleBufferLine: EditorCoreTypes.LineNumber.t,
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

module SignatureHelp: {
  let isActive: model => bool;

  module View: {
    let make:
      (
        ~x: int,
        ~y: int,
        ~theme: Oni_Core.ColorTheme.Colors.t,
        ~tokenTheme: Oni_Syntax.TokenTheme.t,
        ~editorFont: Service_Font.font,
        ~uiFont: Oni_Core.UiFont.t,
        ~languageInfo: Exthost.LanguageInfo.t,
        ~buffer: Oni_Core.Buffer.t,
        ~grammars: Oni_Syntax.GrammarRepository.t,
        ~dispatch: msg => unit,
        ~model: model,
        unit
      ) =>
      Revery.UI.element;
  };
};

module DocumentSymbols: {
  type symbol = {
    uniqueId: string,
    name: string,
    detail: string,
    kind: Exthost.SymbolKind.t,
    range: CharacterRange.t,
    selectionRange: CharacterRange.t,
  };

  type t = list(Tree.t(symbol, symbol));

  let get: (~bufferId: int, model) => option(t);
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
  let keybindings: list(Feature_Input.Schema.keybinding);
};

module Definition: {
  let get: (~bufferId: int, model) => option(Exthost.DefinitionLink.t);

  let getAt:
    (~bufferId: int, ~range: CharacterRange.t, model) =>
    option(Exthost.DefinitionLink.t);

  let isAvailable: (~bufferId: int, model) => bool;
};

module References: {let get: model => list(Exthost.Location.t);};

module DocumentHighlights: {
  let getByLine:
    (~bufferId: int, ~line: int, model) => list(CharacterRange.t);

  let getLinesWithHighlight: (~bufferId: int, model) => list(int);
};

// TODO: Remove
module CompletionMeet = CompletionMeet;
