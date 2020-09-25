open Oni_Core;
open Revery.UI;
open Oni_Components;

open Exthost.Extension;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;
  let container = [flexDirection(`Column), flexGrow(1), overflow(`Hidden)];
  let inputContainer = [margin(12)];

  let resultsContainer = (~isFocused, theme) => {
    let focusColor =
      isFocused
        ? Colors.focusBorder.from(theme) : Revery.Colors.transparentWhite;
    [
      flexDirection(`Column),
      flexGrow(1),
      border(~color=focusColor, ~width=1),
    ];
  };
};

type state = {
  installedExpanded: bool,
  bundledExpanded: bool,
  width: int,
};

let default = {installedExpanded: true, bundledExpanded: false, width: 225};

type msg =
  | InstalledTitleClicked
  | BundledTitleClicked
  | WidthChanged(int);

let reduce = (msg, model) =>
  switch (msg) {
  | InstalledTitleClicked => {
      ...model,
      installedExpanded: !model.installedExpanded,
    }
  | BundledTitleClicked => {...model, bundledExpanded: !model.bundledExpanded}
  | WidthChanged(width) => {...model, width}
  };

let installButton = (~font, ~extensionId, ~dispatch, ()) => {
  <ItemView.ActionButton
    font
    title="Install"
    backgroundColor=Revery.Colors.green
    color=Revery.Colors.white
    onAction={() =>
      dispatch(Model.InstallExtensionClicked({extensionId: extensionId}))
    }
  />;
};

let uninstallButton = (~font, ~extensionId, ~dispatch, ()) => {
  <ItemView.ActionButton
    extensionId
    font
    title="Uninstall"
    backgroundColor=Revery.Colors.red
    color=Revery.Colors.white
    onAction={() =>
      dispatch(Model.UninstallExtensionClicked({extensionId: extensionId}))
    }
  />;
};

let progressButton = (~extensionId, ~font, ~title, ()) => {
  <ItemView.ActionButton
    extensionId
    font
    title
    backgroundColor=Revery.Colors.blue
    color=Revery.Colors.white
    onAction={() => ()}
  />;
};

let%component make =
              (~model, ~theme, ~font: UiFont.t, ~isFocused, ~dispatch, ()) => {
  let%hook ({width, installedExpanded, bundledExpanded}, localDispatch) =
    Hooks.reducer(~initialState=default, reduce);

  let showIcon = width > 300;

  let renderBundled =
      (
        ~availableWidth as _,
        ~index as _,
        ~hovered as _,
        ~selected as _,
        extension: Scanner.ScanResult.t,
      ) => {
    let iconPath = extension.manifest.icon;
    let displayName = Manifest.getDisplayName(extension.manifest);
    let author = extension.manifest.author;
    let version = extension.manifest.version;

    let actionButton = React.empty;

    <ItemView
      actionButton
      width
      iconPath
      theme
      displayName
      author
      version
      isRestartRequired=false
      font
      onClick={_ =>
        dispatch(Model.LocalExtensionSelected({extensionInfo: extension}))
      }
    />;
  };

  let renderInstalled =
      (
        ~availableWidth as _,
        ~index as _,
        ~hovered as _,
        ~selected as _,
        extension: Scanner.ScanResult.t,
      ) => {
    let iconPath = extension.manifest.icon;
    let displayName = Manifest.getDisplayName(extension.manifest);
    let author = extension.manifest.author;
    let version = extension.manifest.version;
    let id = Manifest.identifier(extension.manifest);

    let extensionId = extension.manifest |> Manifest.identifier;
    let isRestartRequired = Model.isRestartRequired(~extensionId, model);
    let actionButton =
      Model.isUninstalling(~extensionId=id, model)
        ? <progressButton extensionId font title="Uninstalling" />
        : <uninstallButton font extensionId dispatch />;

    <ItemView
      actionButton
      width
      iconPath
      theme
      displayName
      author
      version
      isRestartRequired
      font
      showIcon
      onClick={_ =>
        dispatch(Model.LocalExtensionSelected({extensionInfo: extension}))
      }
    />;
  };

  let isInstalledFocused = isFocused && model.focusedWindow == Installed;
  let isBundledFocused = isFocused && model.focusedWindow == Bundled;
  let contents =
    if (Component_InputText.isEmpty(model.searchText)) {
      [
        <Component_Accordion.VimList
          title="Installed"
          expanded={installedExpanded || isInstalledFocused}
          model={Model.ViewModel.installed(model.viewModel)}
          uiFont=font
          render=renderInstalled
          isFocused=isInstalledFocused
          theme
          dispatch={msg =>
            dispatch(ViewModel(Model.ViewModel.Installed(msg)))
          }
          onClick={_ => localDispatch(InstalledTitleClicked)}
        />,
        <Component_Accordion.VimList
          title="Bundled"
          expanded={bundledExpanded || isBundledFocused}
          model={Model.ViewModel.bundled(model.viewModel)}
          uiFont=font
          render=renderBundled
          isFocused=isBundledFocused
          theme
          dispatch={msg =>
            dispatch(ViewModel(Model.ViewModel.Bundled(msg)))
          }
          onClick={_ => localDispatch(BundledTitleClicked)}
        />,
      ]
      |> React.listToElement;
    } else {
      <View
        style={Styles.resultsContainer(
          ~isFocused={isFocused && model.focusedWindow != SearchText},
          theme,
        )}>
        <Component_VimList.View
          isActive={isInstalledFocused || isBundledFocused}
          focusedIndex=None
          theme
          model={Model.ViewModel.searchResults(model.viewModel)}
          dispatch={msg =>
            dispatch(ViewModel(Model.ViewModel.SearchResults(msg)))
          }
          render={(
            ~availableWidth as _,
            ~index as _,
            ~hovered as _,
            ~selected as _,
            summary: Service_Extensions.Catalog.Summary.t,
          ) => {
            let displayName =
              summary |> Service_Extensions.Catalog.Summary.name;
            let extensionId = summary |> Service_Extensions.Catalog.Summary.id;
            let {namespace, version, iconUrl, _}: Service_Extensions.Catalog.Summary.t = summary;
            let author = namespace;

            let isRestartRequired =
              Model.isRestartRequired(~extensionId, model);

            let actionButton =
              Model.isInstalling(~extensionId, model)
                ? <progressButton extensionId title="Installing" font />
                : <installButton extensionId dispatch font extensionId />;
            <ItemView
              actionButton
              width
              iconPath=iconUrl
              theme
              displayName
              isRestartRequired
              author
              version
              font
              showIcon
              onClick={_ =>
                dispatch(
                  Model.RemoteExtensionClicked({extensionId: extensionId}),
                )
              }
            />;
          }}
        />
      </View>;
    };

  let isBusy = Model.isSearchInProgress(model) || Model.isBusy(model);

  <View
    style=Styles.container
    onDimensionsChanged={({width, _}) =>
      localDispatch(WidthChanged(width))
    }>
    <BusyBar theme visible=isBusy />
    <View style=Styles.inputContainer>
      <Component_InputText.View
        model={model.searchText}
        isFocused={isFocused && model.focusedWindow == SearchText}
        fontFamily={font.family}
        fontSize={font.size}
        dispatch={msg => dispatch(Model.SearchText(msg))}
        theme
      />
    </View>
    contents
  </View>;
};
