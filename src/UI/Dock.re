open Revery.UI;

open Oni_Core;
open Oni_Model;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;

module Styles = {
  open Style;

  let container = (~theme: Theme.t, ~offsetX) => [
    top(0),
    bottom(0),
    backgroundColor(theme.activityBarBackground),
    alignItems(`Center),
    transform(Transform.[TranslateX(offsetX)]),
  ];

  let item = [
    height(50),
    width(50),
    justifyContent(`Center),
    alignItems(`Center),
  ];
};

let item = (~onClick, ~theme: Theme.t, ~icon, ()) =>
  <Sneakable onClick style=Styles.item>
    <FontIcon color={theme.activityBarForeground} fontSize=22. icon />
  </Sneakable>;

let onExplorerClick = _ => {
  GlobalContext.current().dispatch(Actions.ActivityBar(FileExplorerClick));
};

let onSearchClick = _ => {
  GlobalContext.current().dispatch(Actions.ActivityBar(SearchClick));
};

let onSCMClick = _ => {
  GlobalContext.current().dispatch(Actions.ActivityBar(SCMClick));
};

let onExtensionsClick = _ => {
  GlobalContext.current().dispatch(Actions.ActivityBar(ExtensionsClick));
};

let animation =
  Revery.UI.Animation.(
    animate(Revery.Time.milliseconds(150))
    |> ease(Easing.ease)
    |> tween(-50.0, 0.)
    |> delay(Revery.Time.milliseconds(75))
  );

let%component make = (~theme, ()) => {
  let%hook (offsetX, _animationState, _reset) = Hooks.animation(animation);

  <View style={Styles.container(~theme, ~offsetX)}>
    <item onClick=onExplorerClick theme icon=FontAwesome.copy />
    <item onClick=onSearchClick theme icon=FontAwesome.search />
    <item onClick=onSCMClick theme icon=FontAwesome.codeBranch />
    <item onClick=onExtensionsClick theme icon=FontAwesome.thLarge />
  </View>;
};
