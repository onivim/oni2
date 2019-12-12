open Revery.UI;
open Oni_Core;

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
