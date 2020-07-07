open Oni_Core;
open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Components;

open Exthost.Extension;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;
  let input = [flexGrow(1), margin(12)];
};

let make = (~model, ~theme, ~font: UiFont.t, ~isFocused, ~dispatch, ()) => {
  let renderItem = (extensions: array(Scanner.ScanResult.t), idx) => {
    let extension = extensions[idx];

    let iconPath = extension.manifest.icon;
    let displayName = Manifest.getDisplayName(extension.manifest);
    let author = extension.manifest.author;
    let version = extension.manifest.version;

    <ItemView iconPath theme displayName author version font />;
  };

  let bundledExtensions =
    Model.getExtensions(~category=Scanner.Bundled, model);

  let userExtensions = Model.getExtensions(~category=Scanner.User, model);

  //let developmentExtensions =
  //Extensions.getExtensions(~category=ExtensionScanner.Development, state.extensions) |> Array.of_list;

  let allExtensions = bundledExtensions @ userExtensions |> Array.of_list;
  //let developmentCount = Array.length(developmentExtensions);

  let contents =
    if (Feature_InputText.isEmpty(model.searchText)) {
      <Accordion
        title="Installed"
        expanded=true
        uiFont=font
        renderItem={renderItem(allExtensions)}
        rowHeight=50
        count={Array.length(allExtensions)}
        focused=None
        theme
      />;
    } else {
      let results =
        Model.searchResults(model)
        |> List.map((summary: Service_Extensions.Catalog.Summary.t) => {
             let displayName =
               summary |> Service_Extensions.Catalog.Summary.name;
             let {namespace as author, version, _}: Service_Extensions.Catalog.Summary.t = summary;

             <ItemView iconPath=None theme displayName author version font />;
           })
        |> Array.of_list;

      <FlatList rowHeight=50 theme focused=None count={Array.length(results)}>
        ...{idx => results[idx]}
      </FlatList>;
    };

  <View
    style=Style.[flexDirection(`Column), flexGrow(1), overflow(`Hidden)]>
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
