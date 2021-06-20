open Oni_Core;
open Model;

type t = {notificationsView: Component_VimList.model(notification)};

[@deriving show]
type msg =
  | VimList(Component_VimList.msg)
  | KeyPressed(string)
  | ClearNotificationsButtonClicked
  | Dismissed({id: int});

type outmsg =
  | Nothing
  | ClearNotifications
  | DismissNotification({id: int});

let commands = _model => {
  Component_VimList.Contributions.commands
  |> List.map(Oni_Core.Command.map(msg => VimList(msg)));
};

let keyPress = key => KeyPressed(key);

let initial = {
  notificationsView:
    Component_VimList.create(~rowHeight=Constants.paneRowHeight),
};

let update = (msg, model) => {
  switch (msg) {
  | VimList(listMsg) =>
    let (notificationsView', _outmsg) =
      Component_VimList.update(listMsg, model.notificationsView);
    ({notificationsView: notificationsView'}, Nothing);

  | ClearNotificationsButtonClicked => (model, ClearNotifications)

  | KeyPressed(key) =>
    let notificationsView' =
      Component_VimList.keyPress(key, model.notificationsView);
    ({notificationsView: notificationsView'}, Nothing);

  | Dismissed({id}) => (model, DismissNotification({id: id}))
  };
};

let set = (notifications: list(notification), model) => {
  let searchText = (notification: notification) => {
    notification.message;
  };
  let notificationsArray = notifications |> Array.of_list;

  let notificationsView' =
    model.notificationsView
    |> Component_VimList.set(~searchText, notificationsArray);
  {notificationsView: notificationsView'};
};

module PaneColors = Colors;

module View = {
  open Revery;
  open Revery.UI;
  open Oni_Components;
  module Item = {
    module Styles = {
      open Style;

      let container = [
        flexDirection(`Row),
        alignItems(`Center),
        paddingHorizontal(10),
        paddingVertical(5),
      ];

      let text = (~foreground) => [
        textWrap(TextWrapping.NoWrap),
        marginLeft(6),
        color(foreground),
      ];

      let message = (~foreground) => [flexGrow(1), ...text(~foreground)];

      let closeButton = [
        alignSelf(`Stretch),
        paddingLeft(5),
        paddingRight(3),
      ];
    };

    let colorFor = (item: notification, ~theme) =>
      switch (item.kind) {
      | Warning => PaneColors.warningBackground.from(theme)
      | Error => PaneColors.errorBackground.from(theme)
      | Info => PaneColors.infoBackground.from(theme)
      };

    let iconFor = (item: notification) =>
      switch (item.kind) {
      | Warning => FontAwesome.exclamationTriangle
      | Error => FontAwesome.exclamationCircle
      | Info => FontAwesome.infoCircle
      };

    let make =
        (~notification: notification, ~theme, ~font: UiFont.t, ~dispatch, ()) => {
      let foreground = Feature_Theme.Colors.foreground.from(theme);

      let icon = () =>
        <FontIcon
          icon={iconFor(notification)}
          fontSize=12.
          color={colorFor(notification, ~theme)}
        />;

      let source = () =>
        switch (notification.source) {
        | Some(text) =>
          let foreground = Color.multiplyAlpha(0.5, foreground);
          <Text
            style={Styles.text(~foreground)}
            fontFamily={font.family}
            fontSize=11.
            text
          />;
        | None => React.empty
        };

      let closeButton = () => {
        let onClick = () => dispatch(Dismissed({id: notification.id}));

        <Revery.UI.Components.Clickable onClick style=Styles.closeButton>
          <FontIcon icon=FontAwesome.times fontSize=13. color=foreground />
        </Revery.UI.Components.Clickable>;
      };

      <View style=Styles.container>
        <icon />
        <source />
        <Text
          style={Styles.message(~foreground)}
          fontFamily={font.family}
          fontSize=11.
          text={notification.message}
        />
        <closeButton />
      </View>;
    };
  };
  module Styles = {
    open Style;
    let pane = [flexGrow(1), flexDirection(`Row)];
    let noResultsContainer = [
      flexGrow(1),
      alignItems(`Center),
      justifyContent(`Center),
    ];
    let title = (~theme) => [
      color(PaneColors.PanelTitle.activeForeground.from(theme)),
      margin(8),
    ];
  };
  let make =
      (~isFocused: bool, ~model: t, ~theme, ~uiFont: UiFont.t, ~dispatch, ()) => {
    let innerElement =
      if (Component_VimList.count(model.notificationsView) == 0) {
        <View style=Styles.noResultsContainer>
          <Text
            style={Styles.title(~theme)}
            fontFamily={uiFont.family}
            fontSize={uiFont.size}
            text="No notifications."
          />
        </View>;
      } else {
        <Component_VimList.View
          font=uiFont
          isActive=isFocused
          focusedIndex=None
          theme
          model={model.notificationsView}
          dispatch={msg => dispatch(VimList(msg))}
          render={(
            ~availableWidth as _,
            ~index as _,
            ~hovered as _,
            ~selected as _,
            item,
          ) =>
            <Item notification=item font=uiFont theme dispatch />
          }
        />;
      };
    <View style=Styles.pane> innerElement </View>;
  };

  module Buttons = {
    module Sneakable = Feature_Sneak.View.Sneakable;
    module Colors = Feature_Theme.Colors;

    module Styles = {
      open Style;
      let paneButton = [
        width(32),
        alignItems(`Center),
        justifyContent(`Center),
      ];
    };
    let make = (~font as _, ~theme, ~dispatch, ~model as _, ()) => {
      <Sneakable
        sneakId="dismissNotifications"
        onClick={() => dispatch(ClearNotificationsButtonClicked)}
        style=Styles.paneButton>
        <FontIcon
          icon=FontAwesome.bellSlash
          color={Colors.Tab.activeForeground.from(theme)}
          fontSize=12.
        />
      </Sneakable>;
    };
  };
};
