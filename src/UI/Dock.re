open Revery;
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

  let item = (~isHovered, ~isActive, ~theme: Theme.t) => [
    height(50),
    width(50),
    justifyContent(`Center),
    alignItems(`Center),
    borderLeft(
      ~width=2,
      ~color=
        isActive ? theme.activityBarActiveBorder : Colors.transparentWhite,
    ),
    backgroundColor(
      isHovered ? theme.activityBarActiveBackground : Colors.transparentWhite,
    ),
  ];
};

let%component item = (~onClick, ~theme: Theme.t, ~isActive, ~icon, ()) => {
  let%hook (isHovered, setHovered) = Hooks.state(false);
  let onMouseOver = _ => setHovered(_ => true);
  let onMouseOut = _ => setHovered(_ => false);

  <View onMouseOver onMouseOut>
    <Sneakable onClick style={Styles.item(~isHovered, ~isActive, ~theme)}>
      <FontIcon
        color={
          isActive
            ? theme.activityBarForeground : theme.activityBarInactiveForeground
        }
        fontSize=22.
        icon
      />
    </Sneakable>
  </View>;
};

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

let%component make = (~theme, ~sideBar: SideBar.t, ~pane: Pane.t, ()) => {
  let%hook (offsetX, _animationState, _reset) = Hooks.animation(animation);

  let isSidebarVisible = it => SideBar.isVisible(it, sideBar);
  let isPaneVisible = it => Pane.isVisible(it, pane);

  <View style={Styles.container(~theme, ~offsetX)}>
    <item
      onClick=onExplorerClick
      theme
      isActive={isSidebarVisible(FileExplorer)}
      icon=FontAwesome.copy
    />
    <item
      onClick=onSearchClick
      theme
      isActive={isPaneVisible(Search)}
      icon=FontAwesome.search
    />
    <item
      onClick=onSCMClick
      theme
      isActive={isSidebarVisible(SCM)}
      icon=FontAwesome.codeBranch
    />
    <item
      onClick=onExtensionsClick
      theme
      isActive={isSidebarVisible(Extensions)}
      icon=FontAwesome.thLarge
    />
  </View>;
};
