open Revery.UI;
open Oni_Core;
open Oni_Model;

module LocationList = Oni_Components.LocationList;
module Diagnostics = Feature_LanguageSupport.Diagnostics;
module Diagnostic = Feature_LanguageSupport.Diagnostic;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let pane = [flexGrow(1), flexDirection(`Row)];

  let noResultsContainer = [
    flexGrow(1),
    alignItems(`Center),
    justifyContent(`Center),
  ];

  let title = (~theme) => [
    color(Colors.PanelTitle.activeForeground.from(theme)),
    margin(8),
  ];
};

let toLocListItem = (diagWithUri: (Uri.t, Diagnostic.t)) => {
  let (uri, diag) = diagWithUri;
  let file = Uri.toFileSystemPath(uri);
  let location = Diagnostic.(diag.range.start);
  LocationList.{file, location, text: diag.message, highlight: None};
};

let make = (~diagnostics, ~theme, ~uiFont: UiFont.t, ~editorFont, ()) => {
  let items =
    diagnostics
    |> Diagnostics.getAllDiagnostics
    |> List.map(toLocListItem)
    |> Array.of_list;

  let onSelectItem = (item: LocationList.item) =>
    GlobalContext.current().dispatch(
      Actions.OpenFileByPath(item.file, None, Some(item.location)),
    );

  let innerElement =
    if (Array.length(items) == 0) {
      <View style=Styles.noResultsContainer>
        <Text
          style={Styles.title(~theme)}
          fontFamily={uiFont.normal}
          fontSize={uiFont.size}
          text="No problems, yet!"
        />
      </View>;
    } else {
      <LocationList theme uiFont editorFont items onSelectItem />;
    };

  <View style=Styles.pane> innerElement </View>;
};
