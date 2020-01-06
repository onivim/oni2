/*
 * StatusBar.re
 *
 * Container for StatusBar
 */

open EditorCoreTypes;
open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
open Oni_Model;

open Oni_Model.StatusBarModel;

module Option = Utility.Option;
module Animation = Revery.UI.Animation;

module Notifications = {
  open Notification;

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

    let text = (font: UiFont.t) => [
      fontFamily(font.fontFile),
      fontSize(11),
      textWrap(TextWrapping.NoWrap),
      marginLeft(6),
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

  let%component notification = (~item, ~theme, ~font, ()) => {
    let%hook (yOffset, _animationState, _reset) =
      Hooks.animation(Animations.sequence, ~active=true);

    let Theme.{
          notificationInfoBackground,
          notificationInfoForeground,
          notificationSuccessBackground,
          notificationSuccessForeground,
          notificationWarningBackground,
          notificationWarningForeground,
          notificationErrorBackground,
          notificationErrorForeground,
          _,
        } = theme;

    let (icon, background, foreground) =
      switch (item.kind) {
      | Success => (
          FontAwesome.checkCircle,
          notificationSuccessBackground,
          notificationSuccessForeground,
        )
      | Warning => (
          FontAwesome.exclamationTriangle,
          notificationWarningBackground,
          notificationWarningForeground,
        )
      | Error => (
          FontAwesome.exclamationCircle,
          notificationErrorBackground,
          notificationErrorForeground,
        )
      | Info => (
          FontAwesome.infoCircle,
          notificationInfoBackground,
          notificationInfoForeground,
        )
      };

    let icon = () =>
      <FontIcon
        icon
        fontSize=16
        backgroundColor=background
        color=foreground
      />;

    <View style={Styles.container(~background, ~yOffset)}>
      <icon />
      <Text style={Styles.text(font)} text={item.message} />
    </View>;
  };

  let useExpiration = items => {
    let%hook (active, setActive) = Hooks.state([]);
    let%hook (expired, setExpired) = Hooks.ref([]);
    let%hook (time, _reset) = Hooks.timer(~active=active != [], ());

    let (stillActive, freshlyExpired) =
      List.partition(
        ((_, _, t)) => Time.(time - t < Animations.totalDuration),
        active,
      );

    if (freshlyExpired != []) {
      setActive(_ => stillActive);

      freshlyExpired
      |> List.map(((item, _, _)) => item.id)
      |> List.rev_append(expired)
      |> setExpired;
    };

    let%hook () =
      Hooks.effect(
        If((!==), items),
        () => {
          let untracked =
            items
            |> List.filter(item => !List.mem(item.id, expired))
            |> List.filter(item =>
                 !List.exists(((it, _, _)) => it.id == item.id, active)
               );

          if (untracked != []) {
            let init = item => (item, React.Key.create(), time);
            setActive(tracked => List.map(init, untracked) @ tracked);
          };

          // TODO: Garbage collection of expired, but on what condition?

          None;
        },
      );

    stillActive;
  };

  let%component make = (~notifications, ~theme, ~font, ()) => {
    let%hook active = useExpiration(notifications);

    active
    |> List.rev
    |> List.map(((item, key, _)) => <notification key item theme font />)
    |> React.listToElement;
  };
};

module Styles = {
  open Style;

  let text = (uiFont: UiFont.t) => [
    fontFamily(uiFont.fontFile),
    fontSize(11),
    textWrap(TextWrapping.NoWrap),
  ];

  let view = (bgColor, yOffset) => [
    backgroundColor(bgColor),
    flexDirection(`Row),
    flexGrow(1),
    justifyContent(`SpaceBetween),
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
    transform(Transform.[TranslateY(yOffset)]),
  ];

  let sectionGroup = [
    position(`Relative),
    flexDirection(`Row),
    justifyContent(`SpaceBetween),
    flexGrow(1),
  ];

  let section = alignment => [
    flexDirection(`Row),
    justifyContent(alignment),
    flexGrow(alignment == `Center ? 1 : 0),
  ];

  let item = bg => [
    flexDirection(`Column),
    justifyContent(`Center),
    backgroundColor(bg),
    paddingHorizontal(10),
    minWidth(50),
  ];

  let notification = (~background) => [
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
    backgroundColor(background),
  ];
};

let positionToString =
  fun
  | Some((loc: Location.t)) =>
    Printf.sprintf(
      "%n,%n",
      Index.toOneBased(loc.line),
      Index.toOneBased(loc.column),
    )
  | None => "";

let sectionGroup = (~children, ()) =>
  <View style=Styles.sectionGroup> children </View>;

let section = (~children=React.empty, ~align, ()) =>
  <View style={Styles.section(align)}> children </View>;

let item = (~children, ~backgroundColor, ~onClick=?, ()) => {
  let style = Styles.item(backgroundColor);

  switch (onClick) {
  | Some(onClick) => <Clickable onClick style> children </Clickable>
  | None => <View style> children </View>
  };
};

let textItem = (~font, ~theme: Theme.t, ~text, ()) =>
  <item backgroundColor={theme.statusBarBackground}>
    <Text
      style=Style.[
        backgroundColor(theme.statusBarBackground),
        color(theme.statusBarForeground),
        ...Styles.text(font),
      ]
      text
    />
  </item>;

let notificationCount = (~font, ~theme: Theme.t, ~notifications, ()) => {
  let text = notifications |> List.length |> string_of_int;

  <item backgroundColor={theme.statusBarBackground}>
    <View
      style=Style.[
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
      ]>
      <FontIcon
        icon=FontAwesome.bell
        backgroundColor={theme.statusBarBackground}
        color={theme.statusBarForeground}
        margin=4
      />
      <Text
        style=Style.[
          backgroundColor(theme.statusBarBackground),
          color(theme.statusBarForeground),
          ...Styles.text(font),
        ]
        text
      />
    </View>
  </item>;
};

let diagnosticCount = (~font, ~theme: Theme.t, ~diagnostics, ()) => {
  let text = diagnostics |> Diagnostics.count |> string_of_int;

  let onClick = () =>
    GlobalContext.current().dispatch(Actions.StatusBar(DiagnosticsClicked));

  <item backgroundColor={theme.statusBarBackground} onClick>
    <View
      style=Style.[
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
      ]>
      <FontIcon
        icon=FontAwesome.timesCircle
        backgroundColor={theme.statusBarBackground}
        color={theme.statusBarForeground}
        margin=4
      />
      <Text
        style=Style.[
          backgroundColor(theme.statusBarBackground),
          color(theme.statusBarForeground),
          ...Styles.text(font),
        ]
        text
      />
    </View>
  </item>;
};

let modeIndicator = (~font, ~theme, ~mode, ()) => {
  let (background, foreground) = Theme.getColorsForMode(theme, mode);

  <item backgroundColor=background>
    <Text
      style=Style.[
        backgroundColor(background),
        color(foreground),
        ...Styles.text(font),
      ]
      text={Vim.Mode.show(mode)}
    />
  </item>;
};

let transitionAnimation =
  Animation.(
    animate(Time.ms(150)) |> ease(Easing.ease) |> tween(50.0, 0.)
  );

let%component make = (~state: State.t, ()) => {
  let State.{mode, theme, uiFont: font, diagnostics, notifications, _} = state;

  let%hook (yOffset, _animationState, _reset) =
    Hooks.animation(transitionAnimation);

  let toStatusBarElement = (statusBarItem: Item.t) =>
    <textItem font theme text={statusBarItem.text} />;

  let leftItems =
    state.statusBar
    |> List.filter((item: Item.t) => item.alignment == Alignment.Left)
    |> List.map(toStatusBarElement)
    |> React.listToElement;

  let rightItems =
    state.statusBar
    |> List.filter((item: Item.t) => item.alignment == Alignment.Right)
    |> List.map(toStatusBarElement)
    |> React.listToElement;

  let indentation = () => {
    let text =
      Indentation.getForActiveBuffer(state) |> Indentation.toStatusString;

    <textItem font theme text />;
  };

  let fileType = () => {
    let text =
      state
      |> Selectors.getActiveBuffer
      |> Option.bind(Buffer.getFileType)
      |> Option.value(~default="plaintext");

    <textItem font theme text />;
  };

  let position = () => {
    let text =
      state
      |> Selectors.getActiveEditorGroup
      |> Selectors.getActiveEditor
      |> Option.map(Editor.getPrimaryCursor)
      |> positionToString;

    <textItem font theme text />;
  };

  <View style={Styles.view(theme.statusBarBackground, yOffset)}>
    <section align=`FlexStart>
      <notificationCount font theme notifications />
    </section>
    <sectionGroup>
      <section align=`FlexStart> leftItems </section>
      <section align=`FlexStart>
        <diagnosticCount font theme diagnostics />
      </section>
      <section align=`Center />
      <section align=`FlexEnd> rightItems </section>
      <section align=`FlexEnd>
        <indentation />
        <fileType />
        <position />
      </section>
      <Notifications notifications theme font />
    </sectionGroup>
    <section align=`FlexEnd> <modeIndicator font theme mode /> </section>
  </View>;
};
