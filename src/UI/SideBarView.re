open Revery.UI;

open Oni_Model;
module Core = Oni_Core;

open Oni_Model.SideBar;
module Styles = {
  open Style;

  let container = (~bg, ~transition) =>
    Style.[
      backgroundColor(bg),
      width(225),
      transform(Transform.[TranslateX(transition)]),
    ];

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

let animation =
  Revery.UI.Animation.(
    animate(Revery.Time.milliseconds(150))
    |> ease(Easing.easeIn)
    |> tween(-50.0, 0.)
    |> delay(Revery.Time.milliseconds(0))
  );

let%component make = (~state: State.t, ()) => {
  let State.{theme, sideBar, uiFont, _} = state;
  let bg = theme.sideBarBackground;
  let fg = theme.sideBarForeground;

  let sideBarType = SideBar.getType(sideBar);

  let%hook (transition, _animationState, _reset) =
    Hooks.animation(animation, ~active=true);

  let title =
    switch (sideBarType) {
    | FileExplorer => "Explorer"
    | Extensions => "Extensions"
    };

  let elem =
    switch (sideBarType) {
    | FileExplorer => <FileExplorerView state />
    | Extensions => React.empty
    };

  <View style={Styles.container(~bg, ~transition)}>
    <View style={Styles.heading(theme)}>
      <Text text=title style={Styles.title(~fg, ~bg, ~font=uiFont)} />
    </View>
    elem
  </View>;
};
