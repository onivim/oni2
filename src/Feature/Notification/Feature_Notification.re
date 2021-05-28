module PaneColors = Colors;
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

include Model;
module Colors = PaneColors;

[@deriving show({with_path: false})]
type internal = {
  id: int,
  kind,
  message: string,
  source: option(string),
  ephemeral: bool,
  yOffsetAnimation: [@opaque] option(Component_Animation.t(float)),
};

let internalToExternal: internal => notification =
  (internal: internal) => {
    {
      id: internal.id,
      kind: internal.kind,
      message: internal.message,
      source: internal.source,
      ephemeral: internal.ephemeral,
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
  pane: Pane.t,
};

let initial = {
  all: [],
  activeNotifications: IntSet.empty,
  statusBarBackgroundColor: None,
  statusBarForegroundColor: None,
  pane: Pane.initial,
};

let count = ({all, _}) =>
  all |> List.filter(notification => !notification.ephemeral) |> List.length;

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
      let bg = PaneColors.backgroundFor(notification);
      let fg = PaneColors.foregroundFor(notification);
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

let updatePane = model => {
  let notifications = all(model);
  {...model, pane: model.pane |> Pane.set(notifications)};
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
  | Pane(Pane.msg)
  | AnimateYOffset({
      id: int,
      msg: [@opaque] Component_Animation.msg,
    })
  | AnimateBackground([@opaque] Component_Animation.ColorTransition.msg)
  | AnimateForeground([@opaque] Component_Animation.ColorTransition.msg);

module Msg = {
  let clear = count => Clear({count: count});
};

let dismiss = (~theme, ~config, ~id, model) => {
  {
    ...model,
    all: List.filter(it => it.id != id, model.all),
    activeNotifications: IntSet.remove(id, model.activeNotifications),
  }
  |> updateColorTransition(~config, ~theme)
  |> updatePane;
};

let clear = (~count, ~config, ~theme, model) => {
  let all =
    if (count == 0) {
      [];
    } else {
      Utility.ListEx.firstk(count, model.all);
    };
  {...model, all, activeNotifications: IntSet.empty}
  |> updateColorTransition(~config, ~theme)
  |> updatePane;
};

let update = (~theme, ~config, model, msg) => {
  let animationsEnabled =
    Feature_Configuration.GlobalConfiguration.animation.get(config);
  switch (msg) {
  | Clear({count}) => model |> clear(~count, ~theme, ~config)
  | Created(item) =>
    let yOffsetAnimation =
      animationsEnabled
        ? Some(Component_Animation.make(Animations.sequence)) : None;
    {
      ...model,
      all: [{...item, yOffsetAnimation}, ...model.all],
      activeNotifications: IntSet.add(item.id, model.activeNotifications),
    }
    |> updateColorTransition(~config, ~theme)
    |> updatePane;

  | Dismissed({id}) => model |> dismiss(~theme, ~config, ~id)

  | Expire({id}) =>
    {
      ...model,
      activeNotifications: IntSet.remove(id, model.activeNotifications),
    }
    |> updateColorTransition(~config, ~theme)
    |> updatePane

  | Pane(paneMsg) =>
    let (pane', outmsg) = Pane.update(paneMsg, model.pane);
    let model' = {...model, pane: pane'};
    switch (outmsg) {
    | Nothing => model'
    | DismissNotification({id}) => model' |> dismiss(~theme, ~config, ~id)
    | ClearNotifications => model' |> clear(~theme, ~config, ~count=0)
    };

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
  let create = (~ephemeral=false, ~kind=Info, ~source=?, message) =>
    Isolinear.Effect.createWithDispatch(~name="notification.create", dispatch =>
      if (Oni_Core.Utility.StringEx.isEmpty(message)) {
        let source = source |> Option.value(~default="Unknown");
        Log.warnf(m => m("Received empty notification from %s", source));
      } else {
        dispatch(
          Created({
            id: Internal.generateId(),
            ephemeral,
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
           notification.ephemeral
             ? Dismissed({id: notification.id})
             : Expire({id: notification.id})
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
};

// CONTRIBUTIONS

module Contributions = {
  let colors =
    PaneColors.[
      infoBackground,
      infoForeground,
      warningBackground,
      warningForeground,
      errorBackground,
      errorForeground,
    ];

  let pane: Feature_Pane.Schema.t(model, msg) = {
    let contextKeys = (~isFocused, model) => {
      isFocused
        ? Component_VimList.Contributions.contextKeys(
            model.pane.notificationsView,
          )
        : WhenExpr.ContextKeys.empty;
    };

    Feature_Pane.Schema.(
      panel(
        ~title="Notifications",
        ~id=Some("workbench.panel.notifications"),
        ~buttons=
          (~font, ~theme, ~dispatch, ~model) => {
            <Pane.View.Buttons font theme dispatch model={model.pane} />
          },
        ~commands=Pane.commands,
        ~contextKeys,
        ~sub=(~isFocused as _, _model) => Isolinear.Sub.none,
        ~view=
          (
            ~config as _,
            ~editorFont as _,
            ~font,
            ~isFocused,
            ~iconTheme as _,
            ~languageInfo as _,
            ~workingDirectory as _,
            ~theme,
            ~dispatch,
            ~model,
          ) =>
            <Pane.View
              uiFont=font
              isFocused
              theme
              dispatch
              model={model.pane}
            />,
        ~keyPressed=Pane.keyPress,
      )
      |> map(~model=Fun.id, ~msg=msg => Pane(msg))
    );
  };
};
