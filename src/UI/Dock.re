open Revery.UI;

open Oni_Core;
open Oni_Model;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;
module Colors = Feature_Theme.Colors.ActivityBar;

module Styles = {
  open Style;

  let container = (~theme, ~offsetX) => [
    top(0),
    bottom(0),
    backgroundColor(Colors.background.from(theme)),
    alignItems(`Center),
    transform(Transform.[TranslateX(offsetX)]),
  ];

  let item = (~isHovered, ~isActive, ~theme) => [
    height(50),
    width(50),
    justifyContent(`Center),
    alignItems(`Center),
    borderLeft(
      ~width=2,
      ~color=(isActive ? Colors.activeBorder : Colors.border).from(theme),
    ),
    backgroundColor(
      theme |> (isHovered ? Colors.activeBackground : Colors.background).from,
    ),
  ];
};

let%component item = (~onClick, ~theme, ~isActive, ~icon, ()) => {
  let%hook (isHovered, setHovered) = Hooks.state(false);
  let onMouseOver = _ => setHovered(_ => true);
  let onMouseOut = _ => setHovered(_ => false);
  let iconColor = isActive ? Colors.foreground : Colors.inactiveForeground;

  <View onMouseOver onMouseOut>
    <Sneakable onClick style={Styles.item(~isHovered, ~isActive, ~theme)}>
      <FontIcon color={iconColor.from(theme)} fontSize=22. icon />
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

let%component make =
              (
                ~theme: ColorTheme.resolver,
                ~sideBar: SideBar.t,
                ~pane: Pane.t,
                (),
              ) => {
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
