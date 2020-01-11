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

let useExpiration = (~equals=(==), ~expireAfter, items) => {
  let%hook (active, setActive) = Hooks.state([]);
  let%hook (expired, setExpired) = Hooks.ref([]);
  let%hook (time, _reset) = Hooks.timer(~active=active != [], ());

  let (stillActive, freshlyExpired) =
    List.partition(
      ((_item, activated)) => Time.(time - activated < expireAfter),
      active,
    );

  if (freshlyExpired != []) {
    setActive(_ => stillActive);

    freshlyExpired
    |> List.map(((item, _t)) => item)
    |> List.rev_append(expired)
    |> setExpired;
  };

  let%hook () =
    Hooks.effect(
      If((!==), items),
      () => {
        let untracked =
          items
          |> List.filter(item => !List.exists(equals(item), expired))
          |> List.filter(item =>
               !List.exists(((it, _t)) => equals(it, item), active)
             );

        if (untracked != []) {
          let init = item => (item, time);
          setActive(tracked => List.map(init, untracked) @ tracked);
        };

        // TODO: Garbage collection of expired, but on what condition?

        None;
      },
    );

  List.map(((item, _t)) => item, stillActive);
};

module Notification = {
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

    let text = (~foreground, ~background, font: UiFont.t) => [
      fontFamily(font.fontFile),
      fontSize(11),
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

  let backgroundColorFor = (item, ~theme: Theme.t) =>
    switch (item.kind) {
    | Success => theme.notificationSuccessBackground
    | Warning => theme.notificationWarningBackground
    | Error => theme.notificationErrorBackground
    | Info => theme.notificationInfoBackground
    };

  let foregroundColorFor = (item, ~theme: Theme.t) => {
    switch (item.kind) {
    | Success => theme.notificationSuccessForeground
    | Warning => theme.notificationWarningForeground
    | Error => theme.notificationErrorForeground
    | Info => theme.notificationInfoForeground
    };
  };

  let iconFor = item =>
    switch (item.kind) {
    | Success => FontAwesome.checkCircle
    | Warning => FontAwesome.exclamationTriangle
    | Error => FontAwesome.exclamationCircle
    | Info => FontAwesome.infoCircle
    };

  let%component make = (~item, ~background, ~foreground, ~font, ()) => {
    let%hook (yOffset, _animationState, _reset) =
      Hooks.animation(Animations.sequence, ~active=true);

    let icon = () =>
      <FontIcon
        icon={iconFor(item)}
        fontSize=16
        backgroundColor=background
        color=foreground
      />;

    <View style={Styles.container(~background, ~yOffset)}>
      <icon />
      <Text
        style={Styles.text(~foreground, ~background, font)}
        text={item.message}
      />
    </View>;
  };
};

module Styles = {
  open Style;

  let view = (background, yOffset) => [
    backgroundColor(background),
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

  let text = (~color, ~background, uiFont: UiFont.t) => [
    fontFamily(uiFont.fontFile),
    fontSize(11),
    textWrap(TextWrapping.NoWrap),
    Style.color(color),
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

let item =
    (~children, ~backgroundColor=Colors.transparentWhite, ~onClick=?, ()) => {
  let style = Styles.item(backgroundColor);

  switch (onClick) {
  | Some(onClick) => <Clickable onClick style> children </Clickable>
  | None => <View style> children </View>
  };
};

let textItem = (~background, ~font, ~theme: Theme.t, ~text, ()) =>
  <item>
    <Text
      style={Styles.text(~color=theme.statusBarForeground, ~background, font)}
      text
    />
  </item>;

let notificationCount =
    (~font, ~foreground as color, ~background, ~notifications, ()) => {
  let text = notifications |> List.length |> string_of_int;

  let onClick = () =>
    GlobalContext.current().dispatch(
      Actions.StatusBar(NotificationCountClicked),
    );

  <item onClick>
    <View
      style=Style.[
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
      ]>
      <FontIcon
        icon=FontAwesome.bell
        backgroundColor=background
        color
        margin=4
      />
      <Text style={Styles.text(~color, ~background, font)} text />
    </View>
  </item>;
};

let diagnosticCount = (~font, ~background, ~theme: Theme.t, ~diagnostics, ()) => {
  let color = theme.statusBarForeground;
  let text = diagnostics |> Diagnostics.count |> string_of_int;

  let onClick = () =>
    GlobalContext.current().dispatch(Actions.StatusBar(DiagnosticsClicked));

  <item onClick>
    <View
      style=Style.[
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
      ]>
      <FontIcon
        icon=FontAwesome.timesCircle
        backgroundColor=background
        color
        margin=4
      />
      <Text style={Styles.text(~color, ~background, font)} text />
    </View>
  </item>;
};

let modeIndicator = (~font, ~theme, ~mode, ()) => {
  let (background, foreground) = Theme.getColorsForMode(theme, mode);

  <item backgroundColor=background>
    <Text
      style={Styles.text(~color=foreground, ~background, font)}
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

  let%hook activeNotifications =
    useExpiration(
      ~expireAfter=Notification.Animations.totalDuration,
      ~equals=(a, b) => Oni_Model.Notification.(a.id == b.id),
      notifications,
    );

  let (background, foreground) =
    switch (activeNotifications) {
    | [] => (theme.statusBarBackground, theme.statusBarForeground)
    | [last, ..._] => (
        Notification.backgroundColorFor(last, ~theme),
        Notification.foregroundColorFor(last, ~theme),
      )
    };

  let%hook background =
    CustomHooks.colorTransition(
      ~duration=Notification.Animations.transitionDuration,
      background,
    );
  let%hook foreground =
    CustomHooks.colorTransition(
      ~duration=Notification.Animations.transitionDuration,
      foreground,
    );

  let%hook (yOffset, _animationState, _reset) =
    Hooks.animation(transitionAnimation);

  let toStatusBarElement = (statusBarItem: Item.t) =>
    <textItem font background theme text={statusBarItem.text} />;

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

    <textItem font background theme text />;
  };

  let fileType = () => {
    let text =
      state
      |> Selectors.getActiveBuffer
      |> Option.bind(Buffer.getFileType)
      |> Option.value(~default="plaintext");

    <textItem font background theme text />;
  };

  let position = () => {
    let text =
      state
      |> Selectors.getActiveEditorGroup
      |> Selectors.getActiveEditor
      |> Option.map(Editor.getPrimaryCursor)
      |> positionToString;

    <textItem font background theme text />;
  };

  let notificationPopups = () =>
    activeNotifications
    |> List.rev
    |> List.map(item => <Notification item background foreground font />)
    |> React.listToElement;

  <View style={Styles.view(background, yOffset)}>
    <section align=`FlexStart>
      <notificationCount font foreground background notifications />
    </section>
    <sectionGroup>
      <section align=`FlexStart> leftItems </section>
      <section align=`FlexStart>
        <diagnosticCount font background theme diagnostics />
      </section>
      <section align=`Center />
      <section align=`FlexEnd> rightItems </section>
      <section align=`FlexEnd>
        <indentation />
        <fileType />
        <position />
      </section>
      <notificationPopups />
    </sectionGroup>
    <section align=`FlexEnd> <modeIndicator font theme mode /> </section>
  </View>;
};
