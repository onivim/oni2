open Revery.UI;
open Oni_Core;
open Oni_Model;

module Ext = Oni_Extensions;

module Styles = {
  let pane = Style.[flexGrow(1), flexDirection(`Row)];

  let noResultsContainer =
    Style.[flexGrow(1), alignItems(`Center), justifyContent(`Center)];

  let title = (~theme: Theme.t, ~font: UiFont.t) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      color(theme.editorForeground),
      margin(8),
    ];
};

let toLocListItem = (locationWithUri: Ext.LocationWithUri.t) => {
  let {uri, range}: Ext.LocationWithUri.t = locationWithUri;
  let file = Uri.toFileSystemPath(uri);
  let location = range.start;
  LocationListItem.{file, location, text: "", highlight: None};
};

let make = (~state: State.t, ()) => {
  let {theme, uiFont, editorFont, _}: State.t = state;

  let items = state.references |> List.map(toLocListItem) |> Array.of_list;

  let innerElement =
    if (Array.length(items) == 0) {
      <View style=Styles.noResultsContainer>
        <Text
          style={Styles.title(~theme, ~font=uiFont)}
          text="No references available."
        />
      </View>;
    } else {
      <LocationListView theme uiFont editorFont items />;
    };

  <View style=Styles.pane> innerElement </View>;
};
