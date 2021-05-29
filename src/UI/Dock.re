open Revery.UI;

open Oni_Core;
open Oni_Model;
open Oni_Components;

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

  let container = (~theme) => [
    top(0),
    bottom(0),
    backgroundColor(Colors.background.from(theme)),
    alignItems(`Center),
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

  let notification = (~theme, ~padding as p) => [
    position(`Absolute),
    bottom(4),
    right(4),
    minWidth(15),
    height(16),
    flexDirection(`Row),
    justifyContent(`Center),
    padding(p),
    borderRadius(8.),
    backgroundColor(BadgeColors.background.from(theme)),
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
  let make = (~notification, ~theme, ~font: UiFont.t, ()) => {
    let foregroundColor = BadgeColors.foreground.from(theme);
    let (padding, inner) =
      switch (notification) {
      | Count(count) =>
        let text = count > 999 ? "999+" : string_of_int(count);
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
          2,
          <View style=Style.[marginTop(1), marginLeft(1)]>
            <Codicon icon=Codicon.clock fontSize=10. color=foregroundColor />
          </View>,
        )
      };

    <View style={Styles.notification(~theme, ~padding)}> inner </View>;
  };
};

let%component item =
              (
                ~notification: option(notification)=?,
                ~onClick,
                ~sideBar,
                ~theme,
                ~isActive,
                ~icon: SVGIcon.t,
                ~font: UiFont.t,
                (),
              ) => {
  let%hook (isHovered, setHovered) = Hooks.state(false);
  let onMouseOver = _ => setHovered(_ => true);
  let onMouseOut = _ => setHovered(_ => false);

  let color =
    (isActive ? Colors.foreground : Colors.inactiveForeground).from(theme);

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
      <icon size=24 strokeWidth=2 color />
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

let make =
    (
      ~theme: ColorTheme.Colors.t,
      ~scm: Feature_SCM.model,
      ~sideBar: Feature_SideBar.model,
      ~extensions: Feature_Extensions.model,
      ~font: UiFont.t,
      (),
    ) => {
  let isSidebarVisible = it => Feature_SideBar.isVisible(it, sideBar);

  let extensionNotification =
    Feature_Extensions.isBusy(extensions) ? Some(InProgress) : None;

  let scmCount = Feature_SCM.count(scm);

  let scmNotification = scmCount > 0 ? Some(Count(scmCount)) : None;

  <View style={Styles.container(~theme)}>
    <item
      font
      onClick=onExplorerClick
      sideBar
      theme
      isActive={isSidebarVisible(FileExplorer)}
      icon=FeatherIcons.folder
    />
    <item
      font
      onClick=onSearchClick
      sideBar
      theme
      isActive={isSidebarVisible(Search)}
      icon=FeatherIcons.search
    />
    <item
      font
      onClick=onSCMClick
      sideBar
      theme
      isActive={isSidebarVisible(SCM)}
      icon=FeatherIcons.gitPullRequest
      notification=?scmNotification
    />
    <item
      font
      onClick=onExtensionsClick
      sideBar
      theme
      isActive={isSidebarVisible(Extensions)}
      icon=FeatherIcons.package
      notification=?extensionNotification
    />
  </View>;
};
