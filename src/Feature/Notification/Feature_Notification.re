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
  | Success
  | Info
  | Warning
  | Error;

[@deriving show({with_path: false})]
type notification = {
  id: int,
  kind,
  message: string,
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
  let create = (~kind=Info, message) =>
    Isolinear.Effect.createWithDispatch(~name="notification.create", dispatch =>
      dispatch(Created({id: Internal.generateId(), kind, message}))
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

// VIEW

module View = {
  open Oni_Core;

  open Revery;
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

      let text = (~foreground, ~background, font: UiFont.t) => [
        fontFamily(font.fontFile),
        fontSize(11.),
        textWrap(TextWrapping.NoWrap),
        marginLeft(6),
        color(foreground),
        backgroundColor(background),
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
      | Success => FontAwesome.checkCircle
      | Warning => FontAwesome.exclamationTriangle
      | Error => FontAwesome.exclamationCircle
      | Info => FontAwesome.infoCircle
      };

    let%component make = (~model, ~background, ~foreground, ~font, ()) => {
      let%hook (yOffset, _animationState, _reset) =
        Hooks.animation(Animations.sequence, ~active=true);

      let icon = () =>
        <FontIcon icon={iconFor(model)} fontSize=16. color=foreground />;

      <View style={Styles.container(~background, ~yOffset)}>
        <icon />
        <Text
          style={Styles.text(~foreground, ~background, font)}
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

        let text = (~font: UiFont.t, ~theme: Theme.t) => [
          fontFamily(font.fontFile),
          fontSize(11.),
          textWrap(TextWrapping.NoWrap),
          marginLeft(6),
          color(theme.foreground),
        ];

        let message = (~font, ~theme) => [
          flexGrow(1),
          ...text(~font, ~theme),
        ];

        let closeButton = [alignSelf(`Stretch), paddingHorizontal(5)];
      };

      let colorFor = (item, ~theme: Theme.t) =>
        switch (item.kind) {
        | Success => theme.notificationSuccessBackground
        | Warning => theme.notificationWarningBackground
        | Error => theme.notificationErrorBackground
        | Info => theme.notificationInfoBackground
        };

      let iconFor = item =>
        switch (item.kind) {
        | Success => FontAwesome.checkCircle
        | Warning => FontAwesome.exclamationTriangle
        | Error => FontAwesome.exclamationCircle
        | Info => FontAwesome.infoCircle
        };

      let make = (~item, ~theme: Theme.t, ~font, ~dispatch, ()) => {
        let icon = () =>
          <FontIcon
            icon={iconFor(item)}
            fontSize=16.
            color={colorFor(item, ~theme)}
          />;

        let closeButton = () => {
          let onClick = () => dispatch(Dismissed(item));

          <Clickable onClick style=Styles.closeButton>
            <FontIcon
              icon=FontAwesome.times
              fontSize=13.
              color={theme.foreground}
            />
          </Clickable>;
        };

        <View style=Styles.container>
          <icon />
          <Text style={Styles.message(~font, ~theme)} text={item.message} />
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

      let title = (~theme: Theme.t, ~font: UiFont.t) => [
        fontFamily(font.fontFile),
        fontSize(font.fontSize),
        color(theme.foreground),
        margin(8),
      ];
    };

    let make = (~model, ~theme, ~font, ~dispatch, ()) => {
      let items = model |> List.map(item => <Item item theme font dispatch />);

      let innerElement =
        if (items == []) {
          <View style=Styles.noResultsContainer>
            <Text
              style={Styles.title(~theme, ~font)}
              text="No notifications, yet!"
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
