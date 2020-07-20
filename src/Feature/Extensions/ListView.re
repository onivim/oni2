open Oni_Core;
open Revery.UI;
open Oni_Components;

open Exthost.Extension;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;
  let container = [flexDirection(`Column), flexGrow(1), overflow(`Hidden)];
  let input = [flexGrow(1), margin(12)];
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
    font
    title="Uninstall"
    backgroundColor=Revery.Colors.red
    color=Revery.Colors.white
    onAction={() =>
      dispatch(Model.UninstallExtensionClicked({extensionId: extensionId}))
    }
  />;
};

let%component make =
              (~model, ~theme, ~font: UiFont.t, ~isFocused, ~dispatch, ()) => {
  let%hook ({width, installedExpanded, bundledExpanded}, localDispatch) =
    Hooks.reducer(~initialState=default, reduce);

  let renderBundled = (extensions: array(Scanner.ScanResult.t), idx) => {
    let extension = extensions[idx];

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
      font
    />;
  };

  let renderInstalled = (extensions: array(Scanner.ScanResult.t), idx) => {
    let extension = extensions[idx];

    let iconPath = extension.manifest.icon;
    let displayName = Manifest.getDisplayName(extension.manifest);
    let author = extension.manifest.author;
    let version = extension.manifest.version;

    let actionButton =
      <uninstallButton
        font
        extensionId={extension.manifest |> Manifest.identifier}
        dispatch
      />;

    <ItemView
      actionButton
      width
      iconPath
      theme
      displayName
      author
      version
      font
    />;
  };

  let bundledExtensions =
    Model.getExtensions(~category=Scanner.Bundled, model) |> Array.of_list;

  let userExtensions =
    Model.getExtensions(~category=Scanner.User, model) |> Array.of_list;
  let contents =
    if (Feature_InputText.isEmpty(model.searchText)) {
      [
        <Accordion
          title="Installed"
          expanded=installedExpanded
          uiFont=font
          renderItem={renderInstalled(userExtensions)}
          rowHeight=ItemView.Constants.itemHeight
          count={Array.length(userExtensions)}
          focused=None
          theme
          onClick={_ => localDispatch(InstalledTitleClicked)}
        />,
        <Accordion
          title="Bundled"
          expanded=bundledExpanded
          uiFont=font
          renderItem={renderBundled(bundledExtensions)}
          rowHeight=ItemView.Constants.itemHeight
          count={Array.length(bundledExtensions)}
          focused=None
          theme
          onClick={_ => localDispatch(BundledTitleClicked)}
        />,
      ]
      |> React.listToElement;
    } else {
      let results =
        Model.searchResults(model)
        |> List.map((summary: Service_Extensions.Catalog.Summary.t) => {
             let displayName =
               summary |> Service_Extensions.Catalog.Summary.name;
             let extensionId =
               summary |> Service_Extensions.Catalog.Summary.id;
             let {namespace, version, iconUrl, _}: Service_Extensions.Catalog.Summary.t = summary;
             let author = namespace;

             let actionButton = <installButton dispatch font extensionId />;
             <ItemView
               actionButton
               width
               iconPath=iconUrl
               theme
               displayName
               author
               version
               font
             />;
           })
        |> Array.of_list;

      <FlatList
        rowHeight=ItemView.Constants.itemHeight
        theme
        focused=None
        count={Array.length(results)}>
        ...{idx => results[idx]}
      </FlatList>;
    };

  let isBusy = Model.isSearchInProgress(model) || Model.isBusy(model);

  <View
    style=Styles.container
    onDimensionsChanged={({width, _}) =>
      localDispatch(WidthChanged(width))
    }>
    <BusyBar theme visible=isBusy />
    <Feature_InputText.View
      style=Styles.input
      model={model.searchText}
      isFocused
      fontFamily={font.family}
      fontSize={font.size}
      dispatch={msg => dispatch(Model.SearchText(msg))}
      theme
    />
    contents
  </View>;
};
