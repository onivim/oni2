module Core = Oni_Core;
module Protocol = ExtHostProtocol;
module Workspace = Protocol.Workspace;
module Terminal = ExtHostClient_Terminal;

type t;

// SCM

module SCM: {
  // MODEL

  [@deriving show]
  type command = {
    id: string,
    title: string,
    arguments: list(Core.Json.t),
  };

  module Resource: {
    [@deriving show]
    type t = {
      handle: int,
      uri: Core.Uri.t,
      icons: list(string),
      tooltip: string,
      strikeThrough: bool,
      faded: bool,
      source: option(string),
      letter: option(string),
      color: option(string),
    };
  };

  module ResourceGroup: {
    [@deriving show]
    type t = {
      handle: int,
      id: string,
      label: string,
      hideWhenEmpty: bool,
      resources: list(Resource.t),
    };
  };

  module Provider: {
    [@deriving show]
    type t = {
      handle: int,
      id: string,
      label: string,
      rootUri: option(Core.Uri.t),
      resourceGroups: list(ResourceGroup.t),
      hasQuickDiffProvider: bool,
      count: int,
      commitTemplate: string,
      acceptInputCommand: option(command),
    };
  };

  // UPDATE

  type msg =
    | RegisterSourceControl({
        handle: int,
        id: string,
        label: string,
        rootUri: option(Core.Uri.t),
      })
    | UnregisterSourceControl({handle: int})
    | UpdateSourceControl({
        handle: int,
        hasQuickDiffProvider: option(bool),
        count: option(int),
        commitTemplate: option(string),
        acceptInputCommand: option(command),
      })
    // statusBarCommands: option(_),
    | RegisterSCMResourceGroup({
        provider: int,
        handle: int,
        id: string,
        label: string,
      })
    | UnregisterSCMResourceGroup({
        provider: int,
        handle: int,
      })
    | SpliceSCMResourceStates({
        provider: int,
        group: int,
        start: int,
        deleteCount: int,
        additions: list(Resource.t),
      });

  let handleMessage:
    (~dispatch: msg => unit, string, list(Core.Json.t)) => unit;

  // EFFECTS

  module Effects: {
    let provideOriginalResource:
      (t, list(Provider.t), string, Core.Uri.t => 'msg) =>
      Isolinear.Effect.t('msg);

    let onInputBoxValueChange:
      (t, Provider.t, string) => Isolinear.Effect.t(_);
  };
};

type msg =
  | SCM(SCM.msg)
  | Terminal(Terminal.msg)
  | RegisterTextContentProvider({
      handle: int,
      scheme: string,
    })
  | UnregisterTextContentProvider({handle: int})
  | RegisterDecorationProvider({
      handle: int,
      label: string,
    })
  | UnregisterDecorationProvider({handle: int})
  | DecorationsDidChange({
      handle: int,
      uris: list(Core.Uri.t),
    });

type unitCallback = unit => unit;

let start:
  (
    ~initialConfiguration: Configuration.t,
    ~initData: ExtHostInitData.t=?,
    ~initialWorkspace: Workspace.t=?,
    ~onInitialized: unitCallback=?,
    ~onClosed: unitCallback=?,
    ~onDiagnosticsChangeMany: Protocol.DiagnosticsCollection.t => unit=?,
    ~onDiagnosticsClear: string => unit=?,
    ~onDidActivateExtension: string => unit=?,
    ~onExtensionActivationFailed: string => unit=?,
    ~onTelemetry: string => unit=?,
    ~onOutput: string => unit=?,
    ~onRegisterCommand: string => unit=?,
    ~onRegisterDefinitionProvider: (t, Protocol.BasicProvider.t) => unit=?,
    ~onRegisterDocumentHighlightProvider: (t, Protocol.BasicProvider.t) => unit
                                            =?,
    ~onRegisterDocumentSymbolProvider: (
                                         t,
                                         Protocol.DocumentSymbolProvider.t
                                       ) =>
                                       unit
                                         =?,
    ~onRegisterReferencesProvider: (t, Protocol.BasicProvider.t) => unit=?,
    ~onRegisterSuggestProvider: (t, Protocol.SuggestProvider.t) => unit=?,
    ~onShowMessage: string => unit=?,
    ~onStatusBarSetEntry: ((int, string, int, int)) => unit,
    ~dispatch: msg => unit,
    Core.Setup.t
  ) =>
  t;
let activateByEvent: (string, t) => unit;
let executeContributedCommand:
  (~arguments: list(Core.Json.t)=?, string, t) => unit;
let acceptWorkspaceData: (Workspace.t, t) => unit;
let addDocument: (Protocol.ModelAddedDelta.t, t) => unit;
let updateDocument:
  (Core.Uri.t, Protocol.ModelChangedEvent.t, ~dirty: bool, t) => unit;
let provideCompletions:
  (int, Core.Uri.t, Protocol.OneBasedPosition.t, t) =>
  Lwt.t(option(list(Protocol.SuggestionItem.t)));
let provideDecorations:
  (int, Core.Uri.t, t) => Lwt.t(list(Core.Decoration.t));
let provideDefinition:
  (int, Core.Uri.t, Protocol.OneBasedPosition.t, t) =>
  Lwt.t(Protocol.DefinitionLink.t);
let provideDocumentHighlights:
  (int, Core.Uri.t, Protocol.OneBasedPosition.t, t) =>
  Lwt.t(list(Protocol.DocumentHighlight.t));
let provideDocumentSymbols:
  (int, Core.Uri.t, t) => Lwt.t(list(DocumentSymbol.t));
let provideReferences:
  (int, Core.Uri.t, Protocol.OneBasedPosition.t, t) =>
  Lwt.t(list(LocationWithUri.t));
let provideTextDocumentContent: (int, Core.Uri.t, t) => Lwt.t(string);
let send: (t, Yojson.Safe.t) => unit;
let close: t => unit;

// EFFECTS

module Effects: {
  let executeContributedCommand:
    (t, ~arguments: list(Core.Json.t)=?, string) => Isolinear.Effect.t(_);
};
