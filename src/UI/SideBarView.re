open Revery.UI;
open Revery.UI.Components;

open Oni_Model;
module Core = Oni_Core;

open Oni_Model.SideBar;
module Styles = {
  open Style;

  let container = (~bg) => Style.[backgroundColor(bg), width(225)];

  let title = (~fg, ~bg, ~font: Core.UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    backgroundColor(bg),
    color(fg),
  ];

  let heading = (theme: Core.Theme.t) => [
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(theme.sideBarBackground),
    height(Core.Constants.default.tabHeight),
  ];
};

let make = (~state: State.t, ()) => {
  let State.{theme, sideBar, uiFont as font, _} = state;
  let bg = theme.sideBarBackground;
  let fg = theme.sideBarForeground;

  let sideBarType = SideBar.getType(sideBar);

  let title = switch(sideBarType) {
  | FileExplorer => "Explorer"
  | Extensions => "Extensions"
  };

  let elem = switch(sideBarType) {
  | FileExplorer => <FileExplorerView state />
  | Extensions => React.empty
  };
  

  <View style={Styles.container(bg)}>
    <View style={Styles.heading(theme)}>
      <Text text=title style={Styles.title(~fg, ~bg, ~font)} />
    </View>
    {elem}
  </View>;
};
