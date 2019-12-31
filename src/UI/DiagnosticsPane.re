open Revery.UI;
open Oni_Core;
open Oni_Model;

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

let toLocListItem = (diagWithUri: (Uri.t, Diagnostic.t)) => {
  let (uri, diag) = diagWithUri;
  let file = Uri.toFileSystemPath(uri);
  let location = Diagnostic.(diag.range.start);
  LocationListItem.{file, location, text: diag.message, highlight: None};
};

let make = (~state: State.t, ()) => {
  let {theme, uiFont, editorFont, _}: State.t = state;

  let items =
    state.diagnostics
    |> Diagnostics.getAllDiagnostics
    |> List.map(toLocListItem)
    |> Array.of_list;

  let innerElement =
    if (Array.length(items) == 0) {
      <View style=Styles.noResultsContainer>
        <Text
          style={Styles.title(~theme, ~font=uiFont)}
          text="No problems, yet!"
        />
      </View>;
    } else {
      <LocationListView theme uiFont editorFont items />;
    };

  <View style=Styles.pane> innerElement </View>;
};
