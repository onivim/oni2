open EditorCoreTypes;
open Revery.UI;
open Oni_Core;
open Oni_Model;

module Model = {
  type nonrec t =
  | State(State.t)
  | Buffer(Buffer.t, list(Diagnostic.t), bool)
  | Diagnostic(Diagnostic.t)

  let children = fun
  | State(state) => []
  | Buffer(_, children, _) => children
  | Diagnostic(_) => [];

  let kind = fun
  | State(_) => `Node(`Open)
  | Buffer(_, _, true) => `Node(`Open)
  | Buffer(_, _, false) => `Node(`Closed)
  | Diagnostic(_) => `Leaf;

  let expandedSubtreeSize = fun
  | State(children) => 0 // TODO: List.length(children)
  | Buffer(_, children, _) => List.length(children)
  | Diagnostic(_) => 0
};

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
  LocationListItem.{
    file,
    location,
    text: diag.message,
    highlight: None,
  }
}

let make = (~state: State.t, ()) => {

  let { theme, uiFont, editorFont, _}: State.t = state;


  let items = state.diagnostics
  |> Diagnostics.getAllDiagnostics
  |> List.map(toLocListItem)
  |> Array.of_list;

  print_endline ("RENDERING!! Count: " ++ string_of_int(Array.length(items)));
  let innerElement = if (Array.length(items) == 0) {
    <View style=Styles.noResultsContainer>
      <Text
        style={Styles.title(~theme, ~font=uiFont)}
        text="No problems, yet!"
      />
    </View>;
  } else {
    <LocationListView theme uiFont editorFont items />
  };


  <View style=Styles.pane>
    {innerElement}
  </View>;
};
