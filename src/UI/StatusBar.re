/*
 * StatusBar.re
 *
 * Container for StatusBar
 */

open EditorCoreTypes;
open Revery;
open Revery.UI;

open Oni_Core;
open Oni_Model;

open Oni_Model.StatusBarModel;

module Option = Utility.Option;

module Notification = {
  include Notification;

  module Styles = {
    open Style;

    let container = (~background) => [
      position(`Absolute),
      top(0),
      bottom(0),
      left(0),
      right(0),
      backgroundColor(background),
      flexDirection(`Row),
      alignItems(`Center),
      paddingHorizontal(10),
    ];

    let text = (font: UiFont.t) => [
      fontFamily(font.fontFile),
      fontSize(11),
      textWrap(TextWrapping.NoWrap),
      marginLeft(6),
    ];
  };

  let make = (~item, ~theme, ~font, ()) => {
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
        fontFamily={Constants.default.fontAwesomeSolidPath}
        icon
        fontSize=16
        backgroundColor=background
        color=foreground
      />;

    <View style={Styles.container(~background)}>
      <icon />
      <Text style={Styles.text(font)} text={item.message} />
    </View>;
  };
};

module Styles = {
  open Style;

  let text = (uiFont: UiFont.t) => [
    fontFamily(uiFont.fontFile),
    fontSize(11),
    textWrap(TextWrapping.NoWrap),
  ];

  let view = (bgColor, transition) => [
    backgroundColor(bgColor),
    flexDirection(`Row),
    flexGrow(1),
    justifyContent(`SpaceBetween),
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
    transform(Transform.[TranslateY(transition)]),
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

let item = (~children, ~backgroundColor, ()) =>
  <View style={Styles.item(backgroundColor)}> children </View>;

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

let notificationsItem = (~font, ~theme: Theme.t, ~notifications, ()) => {
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

let diagnosticsItem = (~font, ~theme: Theme.t, ~diagnostics, ()) => {
  let text = diagnostics |> Diagnostics.count |> string_of_int;

  <item backgroundColor={theme.statusBarBackground}>
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

let modeItem = (~font, ~theme, ~mode, ()) => {
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

let animation =
  Revery.UI.Animation.(
    animate(Revery.Time.milliseconds(150))
    |> ease(Easing.ease)
    |> tween(50.0, 0.)
    |> delay(Revery.Time.milliseconds(0))
  );

let%component make = (~state: State.t, ()) => {
  let State.{mode, theme, uiFont: font, diagnostics, notifications, _} = state;

  let%hook (transition, _animationState, _reset) =
    Hooks.animation(animation, ~active=true);

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

  let indentationItem = () => {
    let text =
      Indentation.getForActiveBuffer(state) |> Indentation.toStatusString;

    <textItem font theme text />;
  };

  let fileTypeItem = () => {
    let text =
      state
      |> Selectors.getActiveBuffer
      |> Option.bind(Buffer.getFileType)
      |> Option.value(~default="plaintext");

    <textItem font theme text />;
  };

  let positionItem = () => {
    let text =
      state
      |> Selectors.getActiveEditorGroup
      |> Selectors.getActiveEditor
      |> Option.map(Editor.getPrimaryCursor)
      |> positionToString;

    <textItem font theme text />;
  };

  <View style={Styles.view(theme.statusBarBackground, transition)}>
    <section align=`FlexStart>
      <notificationsItem font theme notifications />
    </section>
    <sectionGroup>
      <section align=`FlexStart> leftItems </section>
      <section align=`FlexStart>
        <diagnosticsItem font theme diagnostics />
      </section>
      <section align=`Center />
      <section align=`FlexEnd> rightItems </section>
      <section align=`FlexEnd>
        <indentationItem />
        <fileTypeItem />
        <positionItem />
      </section>
      {notifications
       |> List.map(item => <Notification item theme font />)
       |> React.listToElement}
    </sectionGroup>
    <section align=`FlexEnd> <modeItem font theme mode /> </section>
  </View>;
};
