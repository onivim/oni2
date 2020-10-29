open Revery.UI;

open Oni_Core;
open Oni_Model;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;
module Colors = Feature_Theme.Colors.ActivityBar;
module BadgeColors = Feature_Theme.Colors.ActivityBarBadge;
module Sneakable = Feature_Sneak.View.Sneakable;

open Feature_SideBar;

type notification =
  | InProgress
  | Count(int);

module Styles = {
  open Style;

  let container = (~theme, ~offsetX) => [
    top(0),
    bottom(0),
    backgroundColor(Colors.background.from(theme)),
    alignItems(`Center),
    transform(Transform.[TranslateX(offsetX)]),
  ];

  let item = (~isHovered, ~isActive, ~theme, ~sideBar) => {
    let borderFn =
      switch (Feature_SideBar.location(sideBar)) {
      | Feature_SideBar.Right => borderRight
      | Feature_SideBar.Left => borderLeft
      };

    [
      position(`Relative),
      height(50),
      width(50),
      justifyContent(`Center),
      alignItems(`Center),
      borderFn(
        ~width=2,
        ~color=(isActive ? Colors.activeBorder : Colors.border).from(theme),
      ),
      backgroundColor(
        theme |> (isHovered ? Colors.activeBackground : Colors.background).from,
      ),
    ];
  };

  let notification = (~scale, ~theme, ~padding) => [
    position(`Absolute),
    bottom(4),
    right(4),
    width(15),
    height(15),
    paddingTop(padding),
    paddingLeft(padding),
    borderRadius(8.),
    backgroundColor(BadgeColors.background.from(theme)),
    transform([Revery.UI.Transform.Scale(scale)]),
  ];
};
module Animations = {
  open Revery;
  open Revery.UI.Animation;
  let appear =
    animate(Time.milliseconds(300))
    |> ease(Easing.cubic)
    |> tween(0.5, 1.0);
};

module Notification = {
  let%component make = (~notification, ~theme, ~font: UiFont.t, ()) => {
    let foregroundColor = BadgeColors.foreground.from(theme);
    let (padding, inner) =
      switch (notification) {
      | Count(count) =>
        let text = count > 9 ? "9+" : string_of_int(count);
        (
          2,
          <Text
            text
            fontFamily={font.family}
            fontSize=10.
            style=[Style.color(foregroundColor)]
          />,
        );
      | InProgress => (
          3,
          <Codicon icon=Codicon.clock fontSize=10. color=foregroundColor />,
        )
      };

    let%hook (scale, _state, _reset) =
      Hooks.animation(
        ~name="Notification Appear",
        Animations.appear,
        ~active=true,
      );
    <View style={Styles.notification(~scale, ~theme, ~padding)}>
      inner
    </View>;
  };
};

let%component item =
              (
                ~notification: option(notification)=?,
                ~onClick,
                ~sideBar,
                ~theme,
                ~isActive,
                ~icon,
                ~iconStyle=`Solid,
                ~font: UiFont.t,
                (),
              ) => {
  let%hook (isHovered, setHovered) = Hooks.state(false);
  let onMouseOver = _ => setHovered(_ => true);
  let onMouseOut = _ => setHovered(_ => false);

  let icon = () => {
    let color = isActive ? Colors.foreground : Colors.inactiveForeground;
    let fontFamily =
      switch (iconStyle) {
      | `Solid => FontAwesome.FontFamily.solid
      | `Regular => FontAwesome.FontFamily.regular
      };

    <FontIcon fontFamily color={color.from(theme)} fontSize=22. icon />;
  };

  let notificationElement =
    switch (notification) {
    | None => React.empty
    | Some(notification) => <Notification notification font theme />
    };

  <View onMouseOver onMouseOut>
    <Sneakable
      sneakId="item"
      onClick
      style={Styles.item(~isHovered, ~isActive, ~theme, ~sideBar)}>
      <icon />
      notificationElement
    </Sneakable>
  </View>;
};

let onExplorerClick = _ => {
  GlobalContext.current().dispatch(Actions.SideBar(FileExplorerClicked));
};

let onSearchClick = _ => {
  GlobalContext.current().dispatch(Actions.SideBar(SearchClicked));
};

let onSCMClick = _ => {
  GlobalContext.current().dispatch(Actions.SideBar(SCMClicked));
};

let onExtensionsClick = _ => {
  GlobalContext.current().dispatch(Actions.SideBar(ExtensionsClicked));
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
                ~theme: ColorTheme.Colors.t,
                ~sideBar: Feature_SideBar.model,
                ~extensions: Feature_Extensions.model,
                ~font: UiFont.t,
                (),
              ) => {
  let%hook (offsetX, _animationState, _reset) =
    Hooks.animation(~name="Dock Notification Animation", animation);

  let isSidebarVisible = it => Feature_SideBar.isVisible(it, sideBar);

  let extensionNotification =
    Feature_Extensions.isBusy(extensions) ? Some(InProgress) : None;

  <View style={Styles.container(~theme, ~offsetX)}>
    <item
      font
      onClick=onExplorerClick
      sideBar
      theme
      isActive={isSidebarVisible(FileExplorer)}
      icon=FontAwesome.copy
      iconStyle=`Regular
    />
    <item
      font
      onClick=onSearchClick
      sideBar
      theme
      isActive={isSidebarVisible(Search)}
      icon=FontAwesome.search
    />
    <item
      font
      onClick=onSCMClick
      sideBar
      theme
      isActive={isSidebarVisible(SCM)}
      icon=FontAwesome.codeBranch
    />
    <item
      font
      onClick=onExtensionsClick
      sideBar
      theme
      isActive={isSidebarVisible(Extensions)}
      icon=FontAwesome.thLarge
      notification=?extensionNotification
    />
  </View>;
};
