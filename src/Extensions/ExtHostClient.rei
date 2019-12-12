module Protocol = ExtHostProtocol;
module Workspace = Protocol.Workspace;
module Core = Oni_Core;

type t;

type unitCallback = unit => unit;

let start:
  (
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
    ~onRegisterDefinitionProvider: (t, Protocol.DefinitionProvider.t) => unit=?,
    ~onRegisterSuggestProvider: (t, Protocol.SuggestProvider.t) => unit=?,
    ~onShowMessage: string => unit=?,
    ~onStatusBarSetEntry: ((int, string, int, int)) => unit,
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
let provideDefinition:
  (int, Core.Uri.t, Protocol.OneBasedPosition.t, t) =>
  Lwt.t(option(Protocol.DefinitionLink.t));
let send: (t, Yojson.Safe.t) => unit;
let close: t => unit;
