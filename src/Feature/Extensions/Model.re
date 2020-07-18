open Oni_Core;
open Exthost.Extension;

[@deriving show({with_path: false})]
type msg =
  | Exthost(Exthost.Msg.ExtensionService.msg)
  | Discovered([@opaque] list(Scanner.ScanResult.t))
  | ExecuteCommand({
      command: string,
      arguments: [@opaque] list(Json.t),
    })
  | KeyPressed(string)
  | Pasted(string)
  | SearchQueryResults(Service_Extensions.Query.t)
  | SearchQueryError(string)
  | SearchText(Feature_InputText.msg)
  | UninstallExtensionClicked({extensionId: string})
  | UninstallExtensionSuccess({extensionId: string})
  | UninstallExtensionFailed({
      extensionId: string,
      errorMsg: string,
    })
  | InstallExtensionClicked({extensionId: string})
  | InstallExtensionSuccess({
      extensionId: string,
      scanResult: [@opaque] Scanner.ScanResult.t,
    })
  | InstallExtensionFailed({
      extensionId: string,
      errorMsg: string,
    })
  | LocalExtensionSelected({extensionInfo: [@opaque] Scanner.ScanResult.t});

module Msg = {
  let exthost = msg => Exthost(msg);
  let keyPressed = key => KeyPressed(key);
  let pasted = contents => Pasted(contents);
  let discovered = scanResult => Discovered(scanResult);
};

type outmsg =
  | Nothing
  | Focus
  | Effect(Isolinear.Effect.t(msg))
  | NotifySuccess(string)
  | NotifyFailure(string)
  | OpenExtensionDetails;

module Selected = {
  open Exthost_Extension;
  open Exthost_Extension.Manifest;
  type t =
    | Local(Scanner.ScanResult.t)
    | Remote(Service_Extensions.Catalog.Details.t);

  let logo =
    fun
    | Local(scanResult) => scanResult.manifest.icon
    // TODO
    | Remote(_) => None;

  let displayName =
    fun
    | Local(scanResult) => scanResult.manifest |> Manifest.getDisplayName
    | Remote(_) => "TODO";

  let description =
    fun
    | Local(scanResult) => scanResult.manifest.description
    | Remote(_) => None;
};

type model = {
  selected: option(Selected.t),
  activatedIds: list(string),
  extensions: list(Scanner.ScanResult.t),
  searchText: Feature_InputText.model,
  latestQuery: option(Service_Extensions.Query.t),
  extensionsFolder: option(string),
  pendingInstalls: list(string),
  pendingUninstalls: list(string),
};

let initial = (~extensionsFolder) => {
  selected: None,
  activatedIds: [],
  extensions: [],
  searchText: Feature_InputText.create(~placeholder="Type to search..."),
  latestQuery: None,
  extensionsFolder,
  pendingInstalls: [],
  pendingUninstalls: [],
};

let isBusy = ({pendingInstalls, pendingUninstalls, _}) => {
  pendingInstalls != [] || pendingUninstalls != [];
};

let searchResults = ({latestQuery, _}) =>
  switch (latestQuery) {
  | None => []
  | Some(query) => query |> Service_Extensions.Query.results
  };

let isSearchInProgress = ({latestQuery, _}) => {
  switch (latestQuery) {
  | None => false
  | Some(query) => !(query |> Service_Extensions.Query.isComplete)
  };
};

module Internal = {
  let filterBundled = (scanner: Scanner.ScanResult.t) => {
    let name = scanner.manifest |> Manifest.identifier;

    name == "vscode.typescript-language-features"
    || name == "vscode.css-language-features"
    || name == "jaredkent.laserwave"
    || name == "jaredly.reason-vscode"
    || name == "arcticicestudio.nord-visual-studio-code";
  };
  let markActivated = (id: string, model) => {
    ...model,
    activatedIds: [id, ...model.activatedIds],
  };

  let add = (extensions, model) => {
    ...model,
    extensions: extensions @ model.extensions,
  };

  let addPendingInstall = (~extensionId, model) => {
    ...model,
    pendingInstalls: [extensionId, ...model.pendingInstalls],
  };

  let addPendingUninstall = (~extensionId, model) => {
    ...model,
    pendingUninstalls: [extensionId, ...model.pendingUninstalls],
  };

  let clearPendingInstall = (~extensionId, model) => {
    ...model,
    pendingInstalls:
      List.filter(id => id != extensionId, model.pendingInstalls),
  };

  let clearPendingUninstall = (~extensionId, model) => {
    ...model,
    pendingUninstalls:
      List.filter(id => id != extensionId, model.pendingUninstalls),
  };

  let installed = (~extensionId, ~scanResult, model) => {
    let model' = model |> clearPendingInstall(~extensionId);

    {...model', extensions: [scanResult, ...model'.extensions]};
  };

  let uninstalled = (~extensionId, model) => {
    let model' = model |> clearPendingUninstall(~extensionId);

    {
      ...model',
      extensions:
        List.filter(
          (scanResult: Exthost.Extension.Scanner.ScanResult.t) => {
            scanResult.manifest
            |> Exthost.Extension.Manifest.identifier != extensionId
          },
          model'.extensions,
        ),
    };
  };
};

let getExtensions = (~category, model) => {
  let results =
    model.extensions
    |> List.filter((ext: Scanner.ScanResult.t) => ext.category == category);

  switch (category) {
  | Scanner.Bundled => List.filter(Internal.filterBundled, results)
  | _ => results
  };
};

let checkAndUpdateSearchText = (~previousText, ~newText, ~query) =>
  if (previousText != newText) {
    if (String.length(newText) == 0) {
      None;
    } else {
      Some(Service_Extensions.Query.create(~searchText=newText));
    };
  } else {
    query;
  };

let update = (~extHostClient, msg, model) => {
  switch (msg) {
  | Exthost(ActivateExtension({extensionId, _})) => (
      Internal.markActivated(extensionId, model),
      Nothing,
    )
  | Exthost(WillActivateExtension(_))
  | Exthost(DidActivateExtension(_))
  | Exthost(ExtensionActivationError(_))
  | Exthost(ExtensionRuntimeError(_)) => (model, Nothing)

  | Discovered(extensions) => (Internal.add(extensions, model), Nothing)
  | ExecuteCommand({command, arguments}) => (
      model,
      Effect(
        Service_Exthost.Effects.Commands.executeContributedCommand(
          ~command,
          ~arguments,
          extHostClient,
        ),
      ),
    )
  | KeyPressed(key) =>
    let previousText = model.searchText |> Feature_InputText.value;
    let searchText' = Feature_InputText.handleInput(~key, model.searchText);
    let newText = searchText' |> Feature_InputText.value;
    let latestQuery =
      checkAndUpdateSearchText(
        ~previousText,
        ~newText,
        ~query=model.latestQuery,
      );
    ({...model, searchText: searchText', latestQuery}, Nothing);
  | Pasted(text) =>
    let previousText = model.searchText |> Feature_InputText.value;
    let searchText' = Feature_InputText.paste(~text, model.searchText);
    let newText = searchText' |> Feature_InputText.value;
    let latestQuery =
      checkAndUpdateSearchText(
        ~previousText,
        ~newText,
        ~query=model.latestQuery,
      );
    ({...model, searchText: searchText', latestQuery}, Nothing);
  | SearchText(msg) =>
    let previousText = model.searchText |> Feature_InputText.value;
    let searchText' = Feature_InputText.update(msg, model.searchText);
    let newText = searchText' |> Feature_InputText.value;
    let latestQuery =
      checkAndUpdateSearchText(
        ~previousText,
        ~newText,
        ~query=model.latestQuery,
      );
    ({...model, searchText: searchText', latestQuery}, Focus);
  | SearchQueryResults(queryResults) =>
    queryResults
    |> Service_Extensions.Query.searchText
    == (model.searchText |> Feature_InputText.value)
      ? ({...model, latestQuery: Some(queryResults)}, Nothing)
      : (model, Nothing)
  | SearchQueryError(_queryResults) =>
    // TODO: Error experience?
    ({...model, latestQuery: None}, Nothing)
  | UninstallExtensionClicked({extensionId}) =>
    let toMsg = (
      fun
      | Ok () => UninstallExtensionSuccess({extensionId: extensionId})
      | Error(msg) => UninstallExtensionFailed({extensionId, errorMsg: msg})
    );
    let eff =
      Service_Extensions.Effects.uninstall(
        ~extensionsFolder=model.extensionsFolder,
        ~toMsg,
        extensionId,
      );
    (model |> Internal.addPendingUninstall(~extensionId), Effect(eff));
  | UninstallExtensionSuccess({extensionId}) => (
      model |> Internal.uninstalled(~extensionId),
      NotifyFailure(
        Printf.sprintf("Successfully uninstalled %s", extensionId),
      ),
    )
  | UninstallExtensionFailed({extensionId, errorMsg}) => (
      model |> Internal.clearPendingUninstall(~extensionId),
      NotifyFailure(
        Printf.sprintf(
          "Extension %s failed to uninstall: %s",
          extensionId,
          errorMsg,
        ),
      ),
    )
  | InstallExtensionClicked({extensionId}) =>
    let toMsg = (
      fun
      | Ok(scanResult) => InstallExtensionSuccess({extensionId, scanResult})
      | Error(msg) => InstallExtensionFailed({extensionId, errorMsg: msg})
    );
    let eff =
      Service_Extensions.Effects.install(
        ~extensionsFolder=model.extensionsFolder,
        ~toMsg,
        extensionId,
      );
    (model |> Internal.addPendingInstall(~extensionId), Effect(eff));
  | InstallExtensionSuccess({extensionId, scanResult}) => (
      model |> Internal.installed(~extensionId, ~scanResult),
      NotifySuccess(
        Printf.sprintf(
          "Extension %s was installed successfully and will be activated on restart.",
          extensionId,
        ),
      ),
    )
  | InstallExtensionFailed({extensionId, errorMsg}) => (
      model |> Internal.clearPendingInstall(~extensionId),
      NotifyFailure(
        Printf.sprintf(
          "Extension %s failed to install: %s",
          extensionId,
          errorMsg,
        ),
      ),
    )
  | LocalExtensionSelected({extensionInfo}) => (
      {...model, selected: Some(Selected.Local(extensionInfo))},
      OpenExtensionDetails,
    )
  };
};
