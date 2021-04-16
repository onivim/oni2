open Oni_Core;
open Exthost.Extension;

module ViewModel = {
  [@deriving show]
  type msg =
    | Bundled(Component_VimList.msg)
    | Installed(Component_VimList.msg)
    | SearchResults(Component_VimList.msg);

  type t = {
    bundled: Component_VimList.model(Scanner.ScanResult.t),
    installed: Component_VimList.model(Scanner.ScanResult.t),
    searchResults:
      Component_VimList.model(Service_Extensions.Catalog.Summary.t),
  };

  let initial = {
    bundled: Component_VimList.create(~rowHeight=72),
    installed: Component_VimList.create(~rowHeight=72),
    searchResults: Component_VimList.create(~rowHeight=72),
  };

  let installed = ({installed, _}) => installed;
  let bundled = ({bundled, _}) => bundled;
  let searchResults = ({searchResults, _}) => searchResults;

  let setBundled = (newBundled, viewModel) => {
    ...viewModel,
    bundled: Component_VimList.set(newBundled, viewModel.bundled),
  };

  let setInstalled = (newInstalled, viewModel) => {
    ...viewModel,
    installed: Component_VimList.set(newInstalled, viewModel.installed),
  };

  let setSearchResults = (newSearchResults, viewModel) => {
    ...viewModel,
    searchResults:
      Component_VimList.set(newSearchResults, viewModel.searchResults),
  };

  let update = (msg, viewModel) => {
    switch (msg) {
    | Bundled(msg) =>
      let (bundled, _outmsg) =
        Component_VimList.update(msg, viewModel.bundled);
      {...viewModel, bundled};
    | Installed(msg) =>
      let (installed, _outmsg) =
        Component_VimList.update(msg, viewModel.installed);
      {...viewModel, installed};
    | SearchResults(msg) =>
      let (searchResults, _outmsg) =
        Component_VimList.update(msg, viewModel.searchResults);
      {...viewModel, searchResults};
    };
  };
};

[@deriving show({with_path: false})]
type msg =
  | Exthost(Exthost.Msg.ExtensionService.msg)
  | Languages({
      resolver: [@opaque] Lwt.u(Exthost.Reply.t),
      msg: Exthost.Msg.Languages.msg,
    })
  | Storage({
      resolver: [@opaque] Lwt.u(Exthost.Reply.t),
      msg: Exthost.Msg.Storage.msg,
    })
  | Discovered([@opaque] list(Scanner.ScanResult.t))
  | ExecuteCommand({
      command: string,
      arguments: [@opaque] list(Json.t),
    })
  | KeyPressed(string)
  | Pasted(string)
  | SearchQueryResults(Service_Extensions.Query.t)
  | SearchQueryError(string)
  | SearchText(Component_InputText.msg)
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
  | UpdateExtensionClicked({extensionId: string})
  | SetThemeClicked({extensionId: string})
  | LocalExtensionSelected({extensionInfo: [@opaque] Scanner.ScanResult.t})
  | RemoteExtensionClicked({extensionId: string})
  | RemoteExtensionSelected({
      extensionInfo: Service_Extensions.Catalog.Details.t,
    })
  | RemoteExtensionUnableToFetchDetails({errorMsg: string})
  | VimWindowNav(Component_VimWindows.msg)
  | ViewModel(ViewModel.msg)
  | UpdateCheckSucceeded({
      extensionId: string,
      latestVersion: [@opaque] option(Semver.t),
    })
  | UpdateCheckFailed({
      extensionId: string,
      msg: string,
    });

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
  | NewExtensions(list(Scanner.ScanResult.t))
  | InstallSucceeded({
      extensionId: string,
      contributions: Exthost.Extension.Contributions.t,
    })
  | NotifySuccess(string)
  | NotifyFailure(string)
  | OpenExtensionDetails
  | SelectTheme({themes: list(Exthost.Extension.Contributions.Theme.t)})
  | UnhandledWindowMovement(Component_VimWindows.outmsg);

module RatingInfo = {
  type t = {
    downloadCount: int,
    rating: int,
    ratingCount: int,
  };
};

module Selected = {
  open Exthost_Extension;
  open Exthost_Extension.Manifest;
  type t =
    | Local(Scanner.ScanResult.t)
    | Remote(Service_Extensions.Catalog.Details.t);

  let ratings =
    fun
    | Local(_) => None
    | Remote(details) =>
      RatingInfo.(
        {
          Service_Extensions.Catalog.Details.(
            Some({
              downloadCount: details |> downloadCount,
              rating: details |> averageRating |> int_of_float,
              ratingCount: details |> reviewCount,
            })
          );
        }
      );

  let identifier =
    fun
    | Local(scanResult) => scanResult.manifest |> Manifest.identifier
    | Remote(details) => details.namespace ++ "." ++ details.name;

  let namespace =
    fun
    | Local(scanResult) => scanResult.manifest |> Manifest.publisher
    | Remote(details) => details.namespace;

  let isPublicNamespace =
    fun
    | Local(_) => false
    | Remote({isPublicNamespace, _}) => isPublicNamespace;

  let logo =
    fun
    | Local(scanResult) => scanResult.manifest.icon
    | Remote({iconUrl, _}) => iconUrl;

  let displayName =
    fun
    | Local(scanResult) => scanResult.manifest |> Manifest.getDisplayName
    | Remote({displayName, namespace, name, _}) =>
      displayName |> Option.value(~default=namespace ++ "." ++ name);

  let description =
    fun
    | Local(scanResult) => scanResult.manifest.description
    | Remote({description, _}) => description;

  let readme =
    fun
    | Local(scanResult) =>
      Some(Rench.Path.join(scanResult.path, "README.md"))
    | Remote({readmeUrl, _}) => readmeUrl;

  let version =
    fun
    | Local({manifest, _}) => manifest.version
    | Remote({version, _}) => version;
};

module Effect = {
  let replySuccess = (~resolver) =>
    Isolinear.Effect.create(~name="feature.extensions.replySuccess", () => {
      Lwt.wakeup(resolver, Exthost.Reply.okEmpty)
    });

  let replyJson = (~resolver, json) =>
    Isolinear.Effect.create(~name="feature.extensions.replyJson", () => {
      Lwt.wakeup(resolver, Exthost.Reply.okJson(json))
    });
};

type extensionState = {isRestartRequired: bool};

module Focus = {
  type t =
    | SearchText
    | Installed
    | Bundled;

  let initial = SearchText;

  let moveDown = (~isSearching, focus) => {
    switch (focus) {
    | SearchText => Some(Installed)
    | Installed when isSearching => None
    | Installed => Some(Bundled)
    | Bundled => None
    };
  };

  let moveUp = (~isSearching, focus) => {
    switch (focus) {
    | SearchText => None
    | Installed => Some(SearchText)
    | Bundled when isSearching => None
    | Bundled => Some(Installed)
    };
  };
};

type model = {
  selected: option(Selected.t),
  activatedIds: list(string),
  extensions: list(Scanner.ScanResult.t),
  searchText: Component_InputText.model,
  latestQuery: option(Service_Extensions.Query.t),
  extensionsFolder: option(FpExp.t(FpExp.absolute)),
  pendingInstalls: list(string),
  pendingUninstalls: list(string),
  globalValues: Yojson.Safe.t,
  localValues: Yojson.Safe.t,
  extensionState: StringMap.t(extensionState),
  focusedWindow: Focus.t,
  vimWindowNavigation: Component_VimWindows.model,
  viewModel: ViewModel.t,
  lastSearchHadError: bool,
  lastErrorMessage: option(string),
  // update checks
  extensionsToCheckForUpdates: list(string),
  updateAvailable: StringMap.t(bool),
};

let resetFocus = model => {...model, focusedWindow: Focus.initial};

module Persistence = {
  type t = Yojson.Safe.t;
  let initial = `Assoc([]);

  let codec = Oni_Core.Persistence.Schema.value;

  let get = (~shared, model) =>
    shared ? model.globalValues : model.localValues;
};

let initial = (~workspacePersistence, ~globalPersistence, ~extensionsFolder) => {
  activatedIds: [],
  selected: None,
  extensions: [],
  searchText: Component_InputText.create(~placeholder="Type to search..."),
  latestQuery: None,
  extensionsFolder,
  pendingInstalls: [],
  pendingUninstalls: [],
  globalValues: globalPersistence,
  localValues: workspacePersistence,
  extensionState: StringMap.empty,

  focusedWindow: Focus.initial,
  vimWindowNavigation: Component_VimWindows.initial,

  viewModel: ViewModel.initial,
  lastSearchHadError: false,
  lastErrorMessage: None,

  extensionsToCheckForUpdates: [],
  updateAvailable: StringMap.empty,
};

let hasCheckedForUpdate = (~extensionId, {updateAvailable, _}) => {
  StringMap.find_opt(extensionId |> String.lowercase_ascii, updateAvailable)
  == None;
};

let isSearching = ({searchText, _}) =>
  !Component_InputText.isEmpty(searchText);

let isBusy = ({pendingInstalls, pendingUninstalls, _}) => {
  pendingInstalls != [] || pendingUninstalls != [];
};

let isInstalling = (~extensionId, {pendingInstalls, _}) => {
  pendingInstalls
  |> List.exists(id =>
       String.lowercase_ascii(id) == String.lowercase_ascii(extensionId)
     );
};

let isUpdateAvailable = (~extensionId, {updateAvailable, _}) => {
  let normalizedExtensionId = String.lowercase_ascii(extensionId);
  updateAvailable
  |> StringMap.find_opt(normalizedExtensionId)
  |> Option.value(~default=false);
};

let getExtension = (~extensionId, {extensions, _}) => {
  extensions
  |> List.filter((ext: Exthost.Extension.Scanner.ScanResult.t) =>
       String.equal(
         String.lowercase_ascii(ext.manifest |> Manifest.identifier),
         String.lowercase_ascii(extensionId),
       )
     )
  |> (l => List.nth_opt(l, 0));
};

let themes = (~extensionId, model) => {
  model
  |> getExtension(~extensionId)
  |> Option.map(({manifest, _}: Exthost.Extension.Scanner.ScanResult.t) =>
       manifest.contributes.themes
     );
};

let isInstalled = (~extensionId, model) => {
  let maybe = model |> getExtension(~extensionId);
  Option.is_some(maybe);
};

let canUpdate = (~extensionId, ~maybeVersion, model) => {
  model
  |> getExtension(~extensionId)
  |> Utility.OptionEx.flatMap(ext => {
       // If it's bundled, we don't support updating at this time..
       Exthost.Extension.Scanner.ScanResult.(
         if (ext.category == Exthost.Extension.Scanner.Bundled) {
           Some(false);
         } else {
           let manifest = ext.manifest;
           Utility.OptionEx.map2(
             (remoteVersion, manifestVersion) => {
               Semver.greater_than(remoteVersion, manifestVersion)
             },
             maybeVersion,
             Manifest.(manifest.version),
           );
         }
       )
     })
  |> Option.value(~default=false);
};

let hasThemes = (~extensionId, model) => {
  themes(~extensionId, model)
  |> Option.map(themes => themes != [])
  |> Option.value(~default=false);
};

let isUninstalling = (~extensionId, {pendingUninstalls, _}) => {
  pendingUninstalls |> List.exists(id => id == extensionId);
};

let isRestartRequired = (~extensionId, {extensionState, _}) => {
  extensionState
  |> StringMap.find_opt(extensionId)
  |> Option.map(({isRestartRequired}) => isRestartRequired)
  |> Option.value(~default=false);
};

let searchResults = ({latestQuery, _}) =>
  switch (latestQuery) {
  | None => []
  | Some(query) => query |> Service_Extensions.Query.results
  };

let updateViewModelSearchResults = model => {
  let searchResults = searchResults(model);
  {
    ...model,
    viewModel:
      ViewModel.setSearchResults(
        searchResults |> Array.of_list,
        model.viewModel,
      ),
  };
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
    {...model, activatedIds: [id, ...model.activatedIds]};
  };

  let getExtensions = (~category, model) => {
    let results =
      model.extensions
      |> List.filter((ext: Scanner.ScanResult.t) => ext.category == category);

    switch (category) {
    | Scanner.Bundled => List.filter(filterBundled, results)
    | _ => results
    };
  };
  let syncViewModel = model => {
    let bundled = getExtensions(~category=Scanner.Bundled, model);
    let installed = getExtensions(~category=Scanner.User, model);
    let viewModel =
      model.viewModel
      |> ViewModel.setBundled(bundled |> Array.of_list)
      |> ViewModel.setInstalled(installed |> Array.of_list);

    {...model, viewModel};
  };

  let add = (extensions, model) => {
    // Add any non-built-in extensions to our update check
    let extensionsToCheckForUpdates' =
      extensions
      |> List.fold_left(
           (acc, curr: Scanner.ScanResult.t) =>
             if (curr.category != Exthost.Extension.Scanner.Bundled) {
               let identifier =
                 Exthost.Extension.Manifest.identifier(curr.manifest);
               [identifier, ...acc];
             } else {
               acc;
             },
           model.extensionsToCheckForUpdates,
         );

    {
      ...model,
      extensionsToCheckForUpdates: extensionsToCheckForUpdates',
      extensions: extensions @ model.extensions,
    }
    |> syncViewModel;
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

  let markRestartNeeded = (~extensionId, model) => {
    ...model,
    extensionState:
      StringMap.update(
        extensionId,
        fun
        | None => Some({isRestartRequired: true})
        | Some(_prev) => Some({isRestartRequired: true}),
        model.extensionState,
      ),
  };

  let uninstalled = (~extensionId, model) => {
    let model' = model |> clearPendingUninstall(~extensionId);
    let extensions =
      List.filter(
        (scanResult: Exthost.Extension.Scanner.ScanResult.t) => {
          let identifier =
            Exthost.Extension.Manifest.identifier(scanResult.manifest);
          String.lowercase_ascii(identifier)
          != String.lowercase_ascii(extensionId);
        },
        model'.extensions,
      );

    {...model', extensions} |> syncViewModel;
  };

  let installed = (~extensionId, ~scanResult, model) => {
    model
    // Remove any existing entries, to replace with new one
    |> uninstalled(~extensionId)
    |> clearPendingInstall(~extensionId)
    |> markRestartNeeded(~extensionId)
    |> add([scanResult]);
  };

  let removeFromUpdateCheck = (~extensionId, model) => {
    let extensionsToCheckForUpdates' =
      model.extensionsToCheckForUpdates
      |> List.filter(ext => ext != extensionId);
    {...model, extensionsToCheckForUpdates: extensionsToCheckForUpdates'};
  };

  let markUpdateAvailable =
      (~extensionId, ~latestVersion: option(Semver.t), model) => {
    let maybeCurrentVersion =
      model
      |> getExtension(~extensionId)
      |> Utility.OptionEx.flatMap(
           (ext: Exthost.Extension.Scanner.ScanResult.t) =>
           Exthost.Extension.Manifest.(ext.manifest.version)
         );

    let isUpdateAvailable =
      Utility.OptionEx.map2(
        (newVersion, curVersion) => {
          Semver.greater_than(newVersion, curVersion)
        },
        latestVersion,
        maybeCurrentVersion,
      )
      |> Option.value(~default=false);

    let updateAvailable' =
      model.updateAvailable |> StringMap.add(extensionId, isUpdateAvailable);
    {...model, updateAvailable: updateAvailable'};
  };
};

let getLanguageIds = model => {
  model.extensions
  |> List.map((ext: Scanner.ScanResult.t) =>
       ext.manifest.contributes.languages
     )
  |> List.flatten
  |> List.map((language: Exthost.Extension.Contributions.Language.t) =>
       language.id
     );
};

let getPersistedValue = (~shared, ~key, model) => {
  let store = shared ? model.globalValues : model.localValues;
  [store]
  |> Yojson.Safe.Util.filter_member(key)
  |> (l => List.nth_opt(l, 0));
};

let checkAndUpdateSearchText = (~hasError, ~previousText, ~newText, ~query) =>
  if (previousText != newText) {
    if (String.length(newText) == 0) {
      (false, None);
    } else {
      (false, Some(Service_Extensions.Query.create(~searchText=newText)));
    };
  } else {
    (hasError, query);
  };

let getExtensions = Internal.getExtensions;

let update = (~extHostClient, msg, model) => {
  switch (msg) {
  | Exthost(WillActivateExtension(_)) => (model, Nothing)

  | Exthost(ExtensionRuntimeError({extensionId, errorsJson})) => (
      model,
      NotifyFailure(
        Printf.sprintf(
          "Extension runtime error %s:%s",
          Exthost.ExtensionId.toString(extensionId),
          Yojson.Safe.to_string(`List(errorsJson)),
        ),
      ),
    )

  | Exthost(ActivateExtension({extensionId, _})) => (
      Internal.markActivated(extensionId, model),
      Nothing,
    )
  | Exthost(ExtensionActivationError({error, extensionId})) => (
      model,
      NotifyFailure(
        Printf.sprintf(
          "Error activating extension %s: %s",
          Exthost.ExtensionId.toString(extensionId),
          Exthost.ExtensionActivationError.toString(error),
        ),
      ),
    )
  | Exthost(DidActivateExtension({extensionId, _})) => (
      Internal.markActivated(extensionId, model),
      Nothing,
    )

  | Languages({resolver, msg}) =>
    switch (msg) {
    | GetLanguages =>
      let languages = getLanguageIds(model) |> List.map(str => `String(str));

      let eff = Effect.replyJson(~resolver, `List(languages));
      (model, Effect(eff));

    // TODO: Handle change language API from extension host
    | ChangeLanguage(_) => (model, Nothing)
    }

  | Storage({resolver, msg}) =>
    switch (msg) {
    | GetValue({shared, key}) =>
      let value = getPersistedValue(~shared, ~key, model);

      let eff =
        switch (value) {
        | None => Effect.replyJson(~resolver, `Null)
        | Some(json) => Effect.replyJson(~resolver, json)
        };
      (model, Effect(eff));

    | SetValue({shared, key, value}) =>
      let store = shared ? model.globalValues : model.localValues;
      let store' = Utility.JsonEx.update(key, _ => Some(value), store);
      let model' =
        shared
          ? {...model, globalValues: store'}
          : {...model, localValues: store'};

      let eff = Effect.replySuccess(~resolver);
      (model', Effect(eff));
    }

  | Discovered(extensions) => (
      Internal.add(extensions, model),
      NewExtensions(extensions),
    )

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
    let previousText = model.searchText |> Component_InputText.value;
    let searchText' = Component_InputText.handleInput(~key, model.searchText);
    let newText = searchText' |> Component_InputText.value;
    let (hasError, latestQuery) =
      checkAndUpdateSearchText(
        ~hasError=model.lastSearchHadError,
        ~previousText,
        ~newText,
        ~query=model.latestQuery,
      );
    (
      {
        ...model,
        lastSearchHadError: hasError,
        searchText: searchText',
        latestQuery,
      }
      |> updateViewModelSearchResults,
      Nothing,
    );
  | Pasted(text) =>
    let previousText = model.searchText |> Component_InputText.value;
    let searchText' = Component_InputText.paste(~text, model.searchText);
    let newText = searchText' |> Component_InputText.value;
    let (hasError, latestQuery) =
      checkAndUpdateSearchText(
        ~hasError=model.lastSearchHadError,
        ~previousText,
        ~newText,
        ~query=model.latestQuery,
      );
    (
      {
        ...model,
        lastSearchHadError: hasError,
        searchText: searchText',
        latestQuery,
      }
      |> updateViewModelSearchResults,
      Nothing,
    );
  | SearchText(msg) =>
    let previousText = model.searchText |> Component_InputText.value;
    let (searchText', inputOutmsg) =
      Component_InputText.update(msg, model.searchText);
    let outmsg =
      switch (inputOutmsg) {
      | Component_InputText.Nothing => Nothing
      | Component_InputText.Focus => Focus
      };
    let newText = searchText' |> Component_InputText.value;
    let (hasError, latestQuery) =
      checkAndUpdateSearchText(
        ~hasError=model.lastSearchHadError,
        ~previousText,
        ~newText,
        ~query=model.latestQuery,
      );

    (
      {
        ...model,
        lastSearchHadError: hasError,
        searchText: searchText',
        latestQuery,
      }
      |> updateViewModelSearchResults,
      outmsg,
    );
  | SearchQueryResults(queryResults) =>
    queryResults
    |> Service_Extensions.Query.searchText
    == (model.searchText |> Component_InputText.value)
      ? (
        {...model, latestQuery: Some(queryResults)}
        |> updateViewModelSearchResults,
        Nothing,
      )
      : (model, Nothing)
  | SearchQueryError(err) => (
      {
        ...model,
        lastSearchHadError: true,
        lastErrorMessage: Some(err),
        latestQuery: None,
      },
      Nothing,
    )
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
      NotifySuccess(
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

  | UpdateExtensionClicked({extensionId}) =>
    let toMsg = (
      fun
      | Ok(scanResult) => InstallExtensionSuccess({extensionId, scanResult})
      | Error(msg) => InstallExtensionFailed({extensionId, errorMsg: msg})
    );
    let eff =
      Service_Extensions.Effects.update(
        ~extensionsFolder=model.extensionsFolder,
        ~toMsg,
        extensionId,
      );
    (model |> Internal.addPendingInstall(~extensionId), Effect(eff));

  | InstallExtensionSuccess({extensionId, scanResult}) => (
      model |> Internal.installed(~extensionId, ~scanResult),
      InstallSucceeded({
        extensionId,
        contributions:
          Exthost.Extension.Manifest.(scanResult.manifest.contributes),
      }),
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
  | RemoteExtensionClicked({extensionId}) => (
      model,
      Effect(
        Service_Extensions.Effects.details(
          ~extensionId,
          ~toMsg={
            fun
            | Ok(extensionInfo) =>
              RemoteExtensionSelected({extensionInfo: extensionInfo})
            | Error(errorMsg) =>
              RemoteExtensionUnableToFetchDetails({errorMsg: errorMsg});
          },
        ),
      ),
    )
  | RemoteExtensionSelected({extensionInfo}) => (
      {...model, selected: Some(Selected.Remote(extensionInfo))},
      OpenExtensionDetails,
    )
  | RemoteExtensionUnableToFetchDetails({errorMsg}) => (
      model,
      NotifyFailure(errorMsg),
    )

  | SetThemeClicked({extensionId}) =>
    themes(~extensionId, model)
    |> Option.map(themes => {(model, SelectTheme({themes: themes}))})
    |> Option.value(
         ~default=(
           model,
           NotifyFailure(
             Printf.sprintf("Unable to find extension: %s", extensionId),
           ),
         ),
       )

  | ViewModel(viewModelMsg) => (
      {...model, viewModel: ViewModel.update(viewModelMsg, model.viewModel)},
      Nothing,
    )

  | VimWindowNav(navMsg) =>
    let (windowNav, outmsg) =
      Component_VimWindows.update(navMsg, model.vimWindowNavigation);

    let model' = {...model, vimWindowNavigation: windowNav};
    let isSearching = !Component_InputText.isEmpty(model.searchText);
    let focusedWindow = model'.focusedWindow;

    let (focus, outmsg) =
      switch (outmsg) {
      | Nothing => (focusedWindow, Nothing)
      | FocusLeft => (focusedWindow, UnhandledWindowMovement(outmsg))
      | FocusRight => (focusedWindow, UnhandledWindowMovement(outmsg))
      | FocusDown =>
        switch (Focus.moveDown(~isSearching, focusedWindow)) {
        | None => (focusedWindow, UnhandledWindowMovement(outmsg))
        | Some(focus) => (focus, Nothing)
        }
      | FocusUp =>
        switch (Focus.moveUp(~isSearching, focusedWindow)) {
        | None => (focusedWindow, UnhandledWindowMovement(outmsg))
        | Some(focus) => (focus, Nothing)
        }

      | PreviousTab
      | NextTab => (focusedWindow, Nothing)
      };
    ({...model', focusedWindow: focus}, outmsg);

  // Update checks
  | UpdateCheckSucceeded({extensionId, latestVersion}) =>
    let model' =
      model
      |> Internal.removeFromUpdateCheck(~extensionId)
      |> Internal.markUpdateAvailable(~extensionId, ~latestVersion);
    (model', Nothing);

  | UpdateCheckFailed({extensionId, _}) =>
    let model' = model |> Internal.removeFromUpdateCheck(~extensionId);
    (model', Nothing);
  };
};
