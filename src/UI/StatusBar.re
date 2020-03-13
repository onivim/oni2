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
open Utility;

open Oni_Model.StatusBarModel;

module Animation = Revery.UI.Animation;
module ContextMenu = Oni_Components.ContextMenu;
module CustomHooks = Oni_Components.CustomHooks;
module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;
module Diagnostics = Feature_LanguageSupport.Diagnostics;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module Editor = Feature_Editor.Editor;
module Theme = Feature_Theme;

module Colors = {
  open ColorTheme.Schema;

  let transparent = Colors.transparentWhite;

  module Notification = {
    let successBackground =
      define("notification.successBackground", all(unspecified));
    let successForeground =
      define("notification.successForeground", all(unspecified));
    let infoBackground =
      define("notification.infoBackground", all(unspecified));
    let infoForeground =
      define("notification.infoForeground", all(unspecified));
    let warningBackground =
      define("notification.warningBackground", all(unspecified));
    let warningForeground =
      define("notification.warningForeground", all(unspecified));
    let errorBackground =
      define("notification.errorBackground", all(unspecified));
    let errorForeground =
      define("notification.errorForeground", all(unspecified));

    let backgroundFor = (item: Notification.t) =>
      switch (item.kind) {
      | Success => successBackground
      | Warning => warningBackground
      | Error => errorBackground
      | Info => infoBackground
      };

    let foregroundFor = (item: Notification.t) =>
      switch (item.kind) {
      | Success => successForeground
      | Warning => warningForeground
      | Error => errorForeground
      | Info => infoForeground
      };
  };

  module StatusBar = {
    let background = define("statusBar.background", all(unspecified));
    let foreground = define("statusBar.foreground", all(unspecified));
  };
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

  let%component make = (~item, ~background, ~foreground, ~font, ()) => {
    let%hook (yOffset, _animationState, _reset) =
      Hooks.animation(Animations.sequence, ~active=true);

    let icon = () =>
      <FontIcon icon={iconFor(item)} fontSize=16. color=foreground />;

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
    fontSize(11.),
    textWrap(TextWrapping.NoWrap),
    Style.color(color),
    backgroundColor(background),
  ];

  let textBold = (~color, ~background, font: UiFont.t) => [
    fontFamily(font.fontFileSemiBold),
    fontSize(11.),
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
    (
      ~children,
      ~backgroundColor=Colors.transparent,
      ~onClick=?,
      ~onRightClick=?,
      (),
    ) => {
  let style = Styles.item(backgroundColor);

  // Avoid cursor turning into pointer if there's no mouse interaction available
  if (onClick == None && onRightClick == None) {
    <View style> children </View>;
  } else {
    <Clickable ?onClick ?onRightClick style> children </Clickable>;
  };
};

let textItem = (~background, ~font, ~colorTheme, ~text, ()) =>
  <item>
    <Text
      style={Styles.text(
        ~color=Colors.StatusBar.foreground.from(colorTheme),
        ~background,
        font,
      )}
      text
    />
  </item>;

let notificationCount =
    (
      ~theme,
      ~font,
      ~foreground as color,
      ~background,
      ~notifications,
      ~contextMenu,
      ~onContextMenuItemSelect,
      (),
    ) => {
  let text = notifications |> List.length |> string_of_int;

  let onClick = () =>
    GlobalContext.current().dispatch(
      Actions.StatusBar(NotificationCountClicked),
    );
  let onRightClick = () =>
    GlobalContext.current().dispatch(
      Actions.StatusBar(NotificationsContextMenu),
    );

  let menu = () => {
    let items =
      ContextMenu.[
        {
          label: "Clear All",
          // icon: None,
          data: Actions.ClearNotifications,
        },
        {
          label: "Open",
          // icon: None,
          data: Actions.StatusBar(NotificationCountClicked),
        },
      ];

    <ContextMenu
      orientation=(`Top, `Left)
      offsetX=(-10)
      items
      theme
      font // correct for item padding
      onItemSelect=onContextMenuItemSelect
    />;
  };

  <item onClick onRightClick>
    {contextMenu == State.ContextMenu.NotificationStatusBarItem
       ? <menu /> : React.empty}
    <View
      style=Style.[
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
      ]>
      <View style=Style.[margin(4)]>
        <FontIcon icon=FontAwesome.bell color />
      </View>
      <Text style={Styles.text(~color, ~background, font)} text />
    </View>
  </item>;
};

let diagnosticCount = (~font, ~background, ~colorTheme, ~diagnostics, ()) => {
  let color = Colors.StatusBar.foreground.from(colorTheme);
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
      <View style=Style.[margin(4)]>
        <FontIcon icon=FontAwesome.timesCircle color />
      </View>
      <Text style={Styles.text(~color, ~background, font)} text />
    </View>
  </item>;
};

let modeIndicator = (~font, ~colorTheme, ~mode, ()) => {
  let background = Theme.Colors.Oni.backgroundFor(mode).from(colorTheme);
  let foreground = Theme.Colors.Oni.foregroundFor(mode).from(colorTheme);

  <item backgroundColor=background>
    <Text
      style={Styles.textBold(~color=foreground, ~background, font)}
      text={Vim.Mode.show(mode)}
    />
  </item>;
};

let transitionAnimation =
  Animation.(
    animate(Time.ms(150)) |> ease(Easing.ease) |> tween(50.0, 0.)
  );

let%component make =
              (~state: State.t, ~contextMenu, ~onContextMenuItemSelect, ()) => {
  let State.{
        mode,
        colorTheme,
        theme,
        uiFont: font,
        diagnostics,
        notifications,
        _,
      } = state;

  let colorTheme = Theme.resolver(colorTheme);

  let%hook activeNotifications =
    CustomHooks.useExpiration(
      ~expireAfter=Notification.Animations.totalDuration,
      ~equals=(a, b) => Oni_Model.Notification.(a.id == b.id),
      notifications,
    );

  let (background, foreground) =
    switch (activeNotifications) {
    | [] =>
      Colors.StatusBar.(
        background.from(colorTheme),
        foreground.from(colorTheme),
      )
    | [last, ..._] =>
      Colors.Notification.(
        backgroundFor(last).from(colorTheme),
        foregroundFor(last).from(colorTheme),
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
    <textItem font background colorTheme text={statusBarItem.text} />;

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

    <textItem font background colorTheme text />;
  };

  let fileType = () => {
    let text =
      state
      |> Selectors.getActiveBuffer
      |> OptionEx.flatMap(Buffer.getFileType)
      |> Option.value(~default="plaintext");

    <textItem font background colorTheme text />;
  };

  let position = () => {
    let text =
      state
      |> Selectors.getActiveEditorGroup
      |> Selectors.getActiveEditor
      |> Option.map(Editor.getPrimaryCursor)
      |> positionToString;

    <textItem font background colorTheme text />;
  };

  let notificationPopups = () =>
    activeNotifications
    |> List.rev
    |> List.map(item => <Notification item background foreground font />)
    |> React.listToElement;

  <View style={Styles.view(background, yOffset)}>
    <section align=`FlexStart>
      <notificationCount
        theme
        font
        foreground
        background
        notifications
        contextMenu
        onContextMenuItemSelect
      />
    </section>
    <sectionGroup>
      <section align=`FlexStart> leftItems </section>
      <section align=`FlexStart>
        <diagnosticCount font background colorTheme diagnostics />
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
    <section align=`FlexEnd> <modeIndicator font colorTheme mode /> </section>
  </View>;
};
