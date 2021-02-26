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
  statusBarBackgroundColor: option(Component_Animation.ColorTransition.t),
  statusBarForegroundColor: option(Component_Animation.ColorTransition.t),
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

let initial = {
  all: [],
  activeNotifications: IntSet.empty,
  statusBarBackgroundColor: None,
  statusBarForegroundColor: None,
};

let statusBarBackground = (~theme, {statusBarBackgroundColor, _}) => {
  statusBarBackgroundColor
  |> Option.map(Component_Animation.ColorTransition.get)
  |> Option.value(
       ~default=Feature_Theme.Colors.StatusBar.background.from(theme),
     );
};

let statusBarForeground = (~theme, {statusBarForegroundColor, _}) => {
  statusBarForegroundColor
  |> Option.map(Component_Animation.ColorTransition.get)
  |> Option.value(
       ~default=Feature_Theme.Colors.StatusBar.foreground.from(theme),
     );
};

let all = ({all, _}) => all |> List.map(internalToExternal);

let active = ({all, activeNotifications, _}) => {
  all
  |> List.filter_map(notification =>
       if (IntSet.mem(notification.id, activeNotifications)) {
         Some(internalToExternal(notification));
       } else {
         None;
       }
     );
};

let updateColorTransition = (~config, ~theme, model) => {
  let animate =
    Feature_Configuration.GlobalConfiguration.animation.get(config);
  let (desiredBackground, desiredForeground) = {
    switch (active(model)) {
    | [notification, ..._] =>
      let bg = Colors.backgroundFor(notification);
      let fg = Colors.foregroundFor(notification);
      (bg.from(theme), fg.from(theme));
    | [] => (
        Feature_Theme.Colors.StatusBar.background.from(theme),
        Feature_Theme.Colors.StatusBar.foreground.from(theme),
      )
    };
  };

  let currentBackground = statusBarBackground(~theme, model);
  let currentForeground = statusBarForeground(~theme, model);

  if (!Revery.Color.equals(desiredBackground, currentBackground)
      || !Revery.Color.equals(desiredForeground, currentForeground)) {
    let delay = Revery.Time.zero;
    let duration = Revery.Time.milliseconds(300);
    let instant = !animate;
    {
      ...model,
      statusBarBackgroundColor:
        Some(
          Component_Animation.ColorTransition.make(
            ~duration,
            ~delay,
            currentBackground,
          )
          |> Component_Animation.ColorTransition.set(
               ~instant,
               ~color=desiredBackground,
             ),
        ),
      statusBarForegroundColor:
        Some(
          Component_Animation.ColorTransition.make(
            ~duration,
            ~delay,
            currentBackground,
          )
          |> Component_Animation.ColorTransition.set(
               ~instant,
               ~color=desiredForeground,
             ),
        ),
    };
  } else {
    model;
  };
};

let changeTheme = updateColorTransition;

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
  | Clear({count: int})
  | AnimateYOffset({
      id: int,
      msg: [@opaque] Component_Animation.msg,
    })
  | AnimateBackground([@opaque] Component_Animation.ColorTransition.msg)
  | AnimateForeground([@opaque] Component_Animation.ColorTransition.msg);

module Msg = {
  let clear = count => Clear({count: count});
};

let update = (~theme, ~config, model, msg) => {
  let animationsEnabled =
    Feature_Configuration.GlobalConfiguration.animation.get(config);
  switch (msg) {
  | Clear({count}) =>
    let all =
      if (count == 0) {
        [];
      } else {
        Utility.ListEx.firstk(count, model.all);
      };
    {...model, all, activeNotifications: IntSet.empty}
    |> updateColorTransition(~config, ~theme);
  | Created(item) =>
    let yOffsetAnimation =
      animationsEnabled
        ? Some(Component_Animation.make(Animations.sequence)) : None;
    {
      ...model,
      all: [{...item, yOffsetAnimation}, ...model.all],
      activeNotifications: IntSet.add(item.id, model.activeNotifications),
    }
    |> updateColorTransition(~config, ~theme);

  | Dismissed({id}) => {
      ...model,
      all: List.filter(it => it.id != id, model.all),
      activeNotifications: IntSet.remove(id, model.activeNotifications),
    }

  | Expire({id}) =>
    {
      ...model,
      activeNotifications: IntSet.remove(id, model.activeNotifications),
    }
    |> updateColorTransition(~config, ~theme)

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

  | AnimateBackground(msg) => {
      ...model,
      statusBarBackgroundColor:
        model.statusBarBackgroundColor
        |> Option.map(Component_Animation.ColorTransition.update(msg)),
    }

  | AnimateForeground(msg) => {
      ...model,
      statusBarForegroundColor:
        model.statusBarForegroundColor
        |> Option.map(Component_Animation.ColorTransition.update(msg)),
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

  let clear = () =>
    Isolinear.Effect.createWithDispatch(~name="notification.clear", dispatch =>
      dispatch(Clear({count: 0}))
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

  let backgroundSub =
    model.statusBarBackgroundColor
    |> Option.map(Component_Animation.ColorTransition.sub)
    |> Option.map(Isolinear.Sub.map(msg => AnimateBackground(msg)))
    |> Option.value(~default=Isolinear.Sub.none);

  let foregroundSub =
    model.statusBarForegroundColor
    |> Option.map(Component_Animation.ColorTransition.sub)
    |> Option.map(Isolinear.Sub.map(msg => AnimateForeground(msg)))
    |> Option.value(~default=Isolinear.Sub.none);

  let animationSub: Isolinear.Sub.t(msg) =
    switch (model.all) {
    | [] => Isolinear.Sub.none
    | [{yOffsetAnimation, id, _}, ..._rest] =>
      yOffsetAnimation
      |> Option.map(Component_Animation.sub)
      |> Option.value(~default=Isolinear.Sub.none)
      |> Isolinear.Sub.map(msg => AnimateYOffset({id, msg}))
    };

  [animationSub, backgroundSub, foregroundSub, ...timerSubs]
  |> Isolinear.Sub.batch;
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

    let colorFor = (item: notification, ~theme) =>
      switch (item.kind) {
      | Warning => Colors.warningBackground.from(theme)
      | Error => Colors.errorBackground.from(theme)
      | Info => Colors.infoBackground.from(theme)
      };

    let iconFor = (item: notification) =>
      switch (item.kind) {
      | Warning => FontAwesome.exclamationTriangle
      | Error => FontAwesome.exclamationCircle
      | Info => FontAwesome.infoCircle
      };

    let make =
        (~notification: notification, ~theme, ~font: UiFont.t, ~onDismiss, ()) => {
      let foreground = Colors.foreground.from(theme);

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
        // TODO: Bring back
        //let onClick = () => dispatch(Dismissed({id: item.id}));

        let onClick = onDismiss;

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
          text={notification.message}
        />
        <closeButton />
      </View>;
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
