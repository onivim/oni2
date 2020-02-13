module Protocol = ExtHostProtocol;
module Workspace = Protocol.Workspace;
module Core = Oni_Core;

type t = ExtHostTransport.t;

type msg =
  | SCM(SCM.msg)
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
let executeContributedCommand: (string, t) => unit;
let acceptWorkspaceData: (Workspace.t, t) => unit;
let addDocument: (Protocol.ModelAddedDelta.t, t) => unit;
let updateDocument:
  (Core.Uri.t, Protocol.ModelChangedEvent.t, bool, t) => unit;
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
