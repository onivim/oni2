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

let make = (~theme, ~uiFont, ()) => {
  <View style=Styles.pane>
    <View style=Styles.noResultsContainer>
      <Text
        style={Styles.title(~theme, ~font=uiFont)}
        text="No problems, yet!"
      />
    </View>
  </View>;
};
