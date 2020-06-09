open Revery;
open Oni_Core;

module Internal = {
  let generateId = {
    let lastId = ref(0);

    () => {
      lastId := lastId^ + 1;
      lastId^;
    };
  };
};

// MODEL

[@deriving show({with_path: false})]
type kind =
  | Info
  | Warning
  | Error;

[@deriving show({with_path: false})]
type notification = {
  id: int,
  kind,
  message: string,
  source: option(string),
};

type model = list(notification);

let initial = [];

// UPDATE

[@deriving show({with_path: false})]
type msg =
  | Created(notification)
  | Dismissed(notification)
  | AllDismissed;

let update = (model, msg) => {
  switch (msg) {
  | Created(item) => [item, ...model]
  | Dismissed(item) => List.filter(it => it.id != item.id, model)
  | AllDismissed => initial
  };
};

// EFFECTS

module Effects = {
  let create = (~kind=Info, ~source=?, message) =>
    Isolinear.Effect.createWithDispatch(~name="notification.create", dispatch =>
      dispatch(Created({id: Internal.generateId(), kind, message, source}))
    );

  let dismiss = notification =>
    Isolinear.Effect.createWithDispatch(~name="notification.dismiss", dispatch =>
      dispatch(Dismissed(notification))
    );

  let dismissAll =
    Isolinear.Effect.createWithDispatch(
      ~name="notification.dismissAll", dispatch =>
      dispatch(AllDismissed)
    );
};

// COLORS

module Colors = {
  open ColorTheme.Schema;
  include Feature_Theme.Colors;

  let infoBackground =
    define("oni.notification.infoBackground", all(hex("#209CEE")));
  let infoForeground =
    define("oni.notification.infoForeground", all(hex("#FFF")));
  let warningBackground =
    define("oni.notification.warningBackground", all(hex("#FFDD57")));
  let warningForeground =
    define("oni.notification.warningForeground", all(hex("#333")));
  let errorBackground =
    define("oni.notification.errorBackground", all(hex("#FF3860")));
  let errorForeground =
    define("oni.notification.errorForeground", all(hex("#FFF")));

  let backgroundFor = notification =>
    switch (notification.kind) {
    | Warning => warningBackground
    | Error => errorBackground
    | Info => infoBackground
    };

  let foregroundFor = notification =>
    switch (notification.kind) {
    | Warning => warningForeground
    | Error => errorForeground
    | Info => infoForeground
    };
};

// VIEW

module View = {
  open Revery.UI;
  open Revery.UI.Components;

  module FontAwesome = Oni_Components.FontAwesome;
  module FontIcon = Oni_Components.FontIcon;

  // POPUP

  module Popup = {
    module Constants = {
      let popupDuration = Time.ms(3000);
    };

    module Styles = {
      open Style;

      let container = (~background, ~yOffset) => [
        position(`Absolute),
        top(0),
        bottom(0),
        left(0),
        right(0),
        backgroundColor(background),
        flexDirection(`Row),
        alignItems(`Center),
        paddingHorizontal(10),
        transform(Transform.[TranslateY(yOffset)]),
      ];

      let text = (~foreground) => [
        textWrap(TextWrapping.NoWrap),
        marginLeft(6),
        color(foreground),
      ];
    };

    module Animations = {
      open Animation;

      let transitionDuration = Time.ms(150);
      let totalDuration =
        Time.(Constants.popupDuration + transitionDuration *. 2.);

      let enter =
        animate(transitionDuration) |> ease(Easing.ease) |> tween(50., 0.);

      let exit =
        animate(transitionDuration) |> ease(Easing.ease) |> tween(0., 50.);

      let sequence =
        enter |> andThen(~next=exit |> delay(Constants.popupDuration));
    };

    let iconFor = item =>
      switch (item.kind) {
      | Warning => FontAwesome.exclamationTriangle
      | Error => FontAwesome.exclamationCircle
      | Info => FontAwesome.infoCircle
      };

    let%component make =
                  (~model, ~background, ~foreground, ~font: UiFont.t, ()) => {
      let%hook (yOffset, _animationState, _reset) =
        Hooks.animation(Animations.sequence, ~active=true);

      let icon = () =>
        <FontIcon icon={iconFor(model)} fontSize=16. color=foreground />;

      let source = () =>
        switch (model.source) {
        | Some(text) =>
          let foreground = Color.multiplyAlpha(0.5, foreground);
          <Text
            style={Styles.text(~foreground)}
            text
            fontFamily={font.family}
            fontSize=11.
          />;
        | None => React.empty
        };

      <View style={Styles.container(~background, ~yOffset)}>
        <icon />
        <source />
        <Text
          style={Styles.text(~foreground)}
          fontFamily={font.family}
          fontSize=11.
          text={model.message}
        />
      </View>;
    };
  };

  // LIST

  module List = {
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

        let closeButton = [alignSelf(`Stretch), paddingHorizontal(5)];
      };

      let colorFor = (item, ~theme) =>
        switch (item.kind) {
        | Warning => Colors.warningBackground.from(theme)
        | Error => Colors.errorBackground.from(theme)
        | Info => Colors.infoBackground.from(theme)
        };

      let iconFor = item =>
        switch (item.kind) {
        | Warning => FontAwesome.exclamationTriangle
        | Error => FontAwesome.exclamationCircle
        | Info => FontAwesome.infoCircle
        };

      let make = (~item, ~theme, ~font: UiFont.t, ~dispatch, ()) => {
        let foreground = Colors.foreground.from(theme);

        let icon = () =>
          <FontIcon
            icon={iconFor(item)}
            fontSize=16.
            color={colorFor(item, ~theme)}
          />;

        let source = () =>
          switch (item.source) {
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
          let onClick = () => dispatch(Dismissed(item));

          <Clickable onClick style=Styles.closeButton>
            <FontIcon icon=FontAwesome.times fontSize=13. color=foreground />
          </Clickable>;
        };

        <View style=Styles.container>
          <icon />
          <source />
          <Text
            style={Styles.message(~foreground)}
            fontFamily={font.family}
            fontSize=11.
            text={item.message}
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

      let container = [
        position(`Absolute),
        top(0),
        bottom(0),
        left(0),
        right(0),
      ];

      let title = (~theme) => [
        color(Colors.PanelTitle.activeForeground.from(theme)),
        margin(8),
      ];
    };

    let make = (~model, ~theme, ~font: UiFont.t, ~dispatch, ()) => {
      let items = model |> List.map(item => <Item item theme font dispatch />);

      let innerElement =
        if (items == []) {
          <View style=Styles.noResultsContainer>
            <Text
              style={Styles.title(~theme)}
              text="No notifications, yet!"
              fontFamily={font.family}
              fontSize={font.size}
            />
          </View>;
        } else {
          <ScrollView style=Styles.container>
            {items |> React.listToElement}
          </ScrollView>;
        };

      <View style=Styles.pane> innerElement </View>;
    };
  };
};

// CONTRIBUTIONS

module Contributions = {
  let colors =
    Colors.[
      infoBackground,
      infoForeground,
      warningBackground,
      warningForeground,
      errorBackground,
      errorForeground,
    ];
};
