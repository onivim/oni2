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

let installButton = (~theme, ~font, ~extensionId, ~dispatch, ()) => {
  let backgroundColor = Colors.Button.background.from(theme);
  let color = Colors.Button.foreground.from(theme);
  <ItemView.ActionButton
    font
    title="Install"
    backgroundColor
    color
    onAction={() =>
      dispatch(Model.InstallExtensionClicked({extensionId: extensionId}))
    }
  />;
};

let uninstallButton = (~theme, ~font, ~extensionId, ~dispatch, ()) => {
  let backgroundColor = Colors.Button.background.from(theme);
  let color = Colors.Button.foreground.from(theme);
  <ItemView.ActionButton
    extensionId
    font
    title="Uninstall"
    backgroundColor
    color
    onAction={() =>
      dispatch(Model.UninstallExtensionClicked({extensionId: extensionId}))
    }
  />;
};

let progressButton = (~theme, ~extensionId, ~font, ~title, ()) => {
  let backgroundColor = Colors.Button.secondaryBackground.from(theme);
  let color = Colors.Button.secondaryForeground.from(theme);
  <ItemView.ActionButton
    extensionId
    font
    title
    backgroundColor
    color
    onAction={() => ()}
  />;
};

let installedButton = (~theme, ~extensionId, ~font, ()) => {
  let backgroundColor = Colors.Button.background.from(theme);
  let color = Colors.Button.foreground.from(theme);
  <ItemView.ActionButton
    extensionId
    font
    title="Installed"
    backgroundColor
    color
    onAction={() => ()}
  />;
};

let updateButton = (~theme, ~extensionId, ~font, ~dispatch, ()) => {
  let backgroundColor = Colors.Button.secondaryBackground.from(theme);
  let color = Colors.Button.secondaryForeground.from(theme);
  <ItemView.ActionButton
    extensionId
    font
    title="Update"
    backgroundColor
    color
    onAction={() =>
      dispatch(Model.UpdateExtensionClicked({extensionId: extensionId}))
    }
  />;
};

let versionToString: option(Semver.t) => string =
  semver => {
    semver |> Option.map(Semver.to_string) |> Option.value(~default="0.0.0");
  };

let%component make =
              (
                ~model,
                ~proxy,
                ~theme,
                ~font: UiFont.t,
                ~isFocused,
                ~dispatch,
                (),
              ) => {
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
    let version = versionToString(extension.manifest.version);

    let actionButton = React.empty;

    <ItemView
      actionButton
      proxy
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
    let version = versionToString(extension.manifest.version);
    let id = Manifest.identifier(extension.manifest);

    let extensionId = extension.manifest |> Manifest.identifier;
    let isRestartRequired = Model.isRestartRequired(~extensionId, model);
    let actionButton =
      if (Model.isInstalling(~extensionId, model)) {
        <progressButton theme extensionId title="Updating" font />;
      } else if (Model.isUninstalling(~extensionId=id, model)) {
        <progressButton theme extensionId font title="Uninstalling" />;
      } else if (Model.isUpdateAvailable(~extensionId=id, model)) {
        <updateButton theme font extensionId dispatch />;
      } else {
        <uninstallButton theme font extensionId dispatch />;
      };

    <ItemView
      proxy
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
      let resultsList =
        <Component_VimList.View
          isActive={isInstalledFocused || isBundledFocused}
          font
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
            let maybeVersion = version;
            let version = versionToString(version);
            let author = namespace;

            let isRestartRequired =
              Model.isRestartRequired(~extensionId, model);

            let actionButton =
              if (Model.isInstalled(~extensionId, model)) {
                if (Model.isInstalling(~extensionId, model)) {
                  <progressButton theme extensionId title="Updating" font />;
                } else if (Model.canUpdate(~extensionId, ~maybeVersion, model)) {
                  <updateButton theme extensionId font dispatch />;
                } else {
                  <installedButton theme extensionId font />;
                };
              } else {
                Model.isInstalling(~extensionId, model)
                  ? <progressButton
                      theme
                      extensionId
                      title="Installing"
                      font
                    />
                  : <installButton
                      theme
                      extensionId
                      dispatch
                      font
                      extensionId
                    />;
              };

            <ItemView
              proxy
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
        />;

      let message =
        model.lastErrorMessage |> Option.value(~default="Unknown error");
      let error =
        <View style=Style.[padding(8)]>
          <Text
            fontFamily={font.family}
            fontSize={font.size}
            text=message
            style=Style.[color(Colors.EditorError.foreground.from(theme))]
          />
        </View>;
      <View
        style={Styles.resultsContainer(
          ~isFocused={isFocused && model.focusedWindow != SearchText},
          theme,
        )}>
        {model.lastSearchHadError ? error : resultsList}
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
