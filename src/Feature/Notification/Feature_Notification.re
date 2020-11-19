open Revery;
open Oni_Core;

module Log = (val Log.withNamespace("Feature.Notification"));

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
  yOffset: float,
};

[@deriving show({with_path: false})]
type internal = {
  id: int,
  kind,
  message: string,
  source: option(string),
  yOffsetAnimation: [@opaque] option(Component_Animation.t(float)),
};

let internalToExternal: internal => notification =
  (internal: internal) => {
    {
      id: internal.id,
      kind: internal.kind,
      message: internal.message,
      source: internal.source,
      yOffset:
        internal.yOffsetAnimation
        |> Option.map(Component_Animation.get)
        |> Option.value(~default=0.),
    };
  };

type model = {
  all: list(internal),
  activeNotifications: IntSet.t,
};

let initial = {all: [], activeNotifications: IntSet.empty};

let all = ({all, _}) => all |> List.map(internalToExternal);

let active = ({all, activeNotifications}) => {
  all
  |> List.filter_map(notification =>
       if (IntSet.mem(notification.id, activeNotifications)) {
         Some(internalToExternal(notification));
       } else {
         None;
       }
     );
};

// ANIMATIONS

module Constants = {
  let popupDuration = Time.ms(3000);
};

module Animations = {
  open Revery;
  open Revery.UI;
  open Revery.UI.Animation;

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

// UPDATE

[@deriving show({with_path: false})]
type msg =
  | Created(internal)
  | Dismissed({id: int})
  | Expire({id: int})
  | AnimateYOffset({
      id: int,
      msg: [@opaque] Component_Animation.msg,
    });

let update = (~config, model, msg) => {
  let animationsEnabled =
    Feature_Configuration.GlobalConfiguration.animation.get(config);
  switch (msg) {
  | Created(item) =>
    let yOffsetAnimation =
      animationsEnabled
        ? Some(Component_Animation.make(Animations.sequence)) : None;
    {
      all: [{...item, yOffsetAnimation}, ...model.all],
      activeNotifications: IntSet.add(item.id, model.activeNotifications),
    };
  | Dismissed({id}) => {
      all: List.filter(it => it.id != id, model.all),
      activeNotifications: IntSet.remove(id, model.activeNotifications),
    }
  | Expire({id}) => {
      ...model,
      activeNotifications: IntSet.remove(id, model.activeNotifications),
    }
  | AnimateYOffset({id, msg}) => {
      ...model,
      all:
        model.all
        |> List.map(item =>
             if (item.id == id) {
               {
                 ...item,
                 yOffsetAnimation:
                   item.yOffsetAnimation
                   |> Option.map(Component_Animation.update(msg)),
               };
             } else {
               item;
             }
           ),
    }
  };
};

// EFFECTS

module Effects = {
  let create = (~kind=Info, ~source=?, message) =>
    Isolinear.Effect.createWithDispatch(~name="notification.create", dispatch =>
      if (Oni_Core.Utility.StringEx.isEmpty(message)) {
        let source = source |> Option.value(~default="Unknown");
        Log.warnf(m => m("Received empty notification from %s", source));
      } else {
        dispatch(
          Created({
            id: Internal.generateId(),
            kind,
            message,
            source,
            yOffsetAnimation: None,
          }),
        );
      }
    );

  let dismiss = (notification: notification) =>
    Isolinear.Effect.createWithDispatch(~name="notification.dismiss", dispatch =>
      dispatch(Dismissed({id: notification.id}))
    );
};

let sub = (model: model) => {
  let timerSubs: list(Isolinear.Sub.t(msg)) =
    model
    |> active
    |> List.map((notification: notification) => {
         Service_Time.Sub.once(
           ~uniqueId="Feature_Notification" ++ string_of_int(notification.id),
           ~delay=Animations.totalDuration,
           ~msg=(~current as _) =>
           Expire({id: notification.id})
         )
       });

  let animationSub: Isolinear.Sub.t(msg) =
    switch (model.all) {
    | [] => Isolinear.Sub.none
    | [{yOffsetAnimation, id, _}, ..._rest] =>
      yOffsetAnimation
      |> Option.map(Component_Animation.sub)
      |> Option.value(~default=Isolinear.Sub.none)
      |> Isolinear.Sub.map(msg => AnimateYOffset({id, msg}))
    };

  [animationSub, ...timerSubs] |> Isolinear.Sub.batch;
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

  let backgroundFor = (notification: notification) =>
    switch (notification.kind) {
    | Warning => warningBackground
    | Error => errorBackground
    | Info => infoBackground
    };

  let foregroundFor = (notification: notification) =>
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

    let iconFor = (item: notification) =>
      switch (item.kind) {
      | Warning => FontAwesome.exclamationTriangle
      | Error => FontAwesome.exclamationCircle
      | Info => FontAwesome.infoCircle
      };

    let make =
        (
          ~key=?,
          ~model: notification,
          ~background,
          ~foreground,
          ~font: UiFont.t,
          (),
        ) => {
      let yOffset = model.yOffset;

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

      <View ?key style={Styles.container(~background, ~yOffset)}>
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
          let onClick = () => dispatch(Dismissed({id: item.id}));

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
        backgroundColor(Revery.Colors.magenta),
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
      let items =
        model.all |> List.map(item => <Item item theme font dispatch />);

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
