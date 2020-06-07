open Oni_Core;
open Utility;
open Exthost.Msg.StatusBar;

// MODEL

module Item = {
  [@deriving show]
  type t = {
    id: string,
    priority: int,
    label: Exthost.Label.t,
    alignment: Exthost.Msg.StatusBar.alignment,
    command: option(string),
  };

  let create = (~command=?, ~id, ~priority, ~label, ~alignment=Left, ()) => {
    id,
    priority,
    label,
    alignment,
    command,
  };
};

// MSG

[@deriving show]
type msg =
  | ItemAdded(Item.t)
  | ItemDisposed(string)
  | DiagnosticsClicked
  | NotificationClearAllClicked
  | NotificationCountClicked
  | NotificationsContextMenu
  | ContributedItemClicked({
      id: string,
      command: string,
    });

type model = {
  items: list(Item.t),
};

let initial = {items: []};

// UPDATE

let update = (model, msg) => {
  let removeItemById = (items: list(Item.t), id) =>
    List.filter(si => Item.(si.id) != id, items);

  switch (msg) {
  | ItemAdded(item) =>
    /* Replace the old item with the new one */
    let newItems = removeItemById(model.items, item.id);
    { items: [item, ...newItems]};
  | ItemDisposed(id) => { items: removeItemById(model.items, id)}
  | _ => model
  };
};

// VIEW

open EditorCoreTypes;
open Revery;
open Revery.UI;
open Revery.UI.Components;
module Animation = Revery.UI.Animation;
module ContextMenu = Oni_Components.ContextMenu;
module CustomHooks = Oni_Components.CustomHooks;
module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;
module Label = Oni_Components.Label;
module Diagnostics = Feature_LanguageSupport.Diagnostics;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module Editor = Feature_Editor.Editor;

module Colors = Feature_Theme.Colors;

open Exthost.Msg.StatusBar;

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
    textWrap(TextWrapping.NoWrap),
    Style.color(color),
    backgroundColor(background),
  ];

  let textBold = (~color, ~background, font: UiFont.t) => [
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
      ~backgroundColor=Revery.Colors.transparentWhite,
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

let textItem = (~background, ~font: UiFont.t, ~theme, ~text, ()) =>
  <item>
    <Text
      style={Styles.text(
        ~color=Colors.StatusBar.foreground.from(theme),
        ~background,
        font,
      )}
      fontFamily={font.normal}
      fontSize=11.
      text
    />
  </item>;

let notificationCount =
    (
      ~theme,
      ~font: UiFont.t,
      ~foreground as color,
      ~background,
      ~notifications: Feature_Notification.model,
      ~contextMenu,
      ~onContextMenuItemSelect,
      ~dispatch,
      (),
    ) => {
  let text =
    (notifications :> list(Feature_Notification.notification))
    |> List.length
    |> string_of_int;

  let onClick = () => dispatch(NotificationCountClicked);
  let onRightClick = () => dispatch(NotificationsContextMenu);

  let menu = () => {
    let items =
      ContextMenu.[
        {
          label: "Clear All",
          // icon: None,
          data: NotificationClearAllClicked,
        },
        {
          label: "Open",
          // icon: None,
          data: NotificationCountClicked,
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
    {contextMenu == Feature_ContextMenu.NotificationStatusBarItem
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
      <Text
        style={Styles.text(~color, ~background, font)}
        text
        fontFamily={font.normal}
        fontSize=11.
      />
    </View>
  </item>;
};

let diagnosticCount =
    (~font, ~background, ~theme, ~diagnostics, ~dispatch, ()) => {
  let color = Colors.StatusBar.foreground.from(theme);
  let text = diagnostics |> Diagnostics.count |> string_of_int;

  let onClick = () => dispatch(DiagnosticsClicked);

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
      <Text
        style={Styles.text(~color, ~background, font)}
        text
        fontFamily={font.normal}
        fontSize=11.
      />
    </View>
  </item>;
};

let modeIndicator = (~font: UiFont.t, ~theme, ~mode, ()) => {
  let background = Colors.Oni.backgroundFor(mode).from(theme);
  let foreground = Colors.Oni.foregroundFor(mode).from(theme);

  <item backgroundColor=background>
    <Text
      style={Styles.text(~color=foreground, ~background, font)}
      text={Mode.toString(mode)}
      fontFamily={font.semiBold}
      fontSize=11.
    />
  </item>;
};

let transitionAnimation =
  Animation.(
    animate(Time.ms(150)) |> ease(Easing.ease) |> tween(50.0, 0.)
  );

let indentationToString = (indentation: IndentationSettings.t) => {
  switch (indentation.mode) {
  | Tabs => "Tabs: " ++ string_of_int(indentation.tabSize)
  | Spaces => "Spaces: " ++ string_of_int(indentation.size)
  };
};

module View = {
  let%component make =
                (
                  ~mode: Oni_Core.Mode.t,
                  ~notifications: Feature_Notification.model,
                  ~diagnostics: Diagnostics.t,
                  ~font: UiFont.t,
                  ~contextMenu: Feature_ContextMenu.model,
                  ~onContextMenuItemSelect,
                  ~activeBuffer: option(Oni_Core.Buffer.t),
                  ~activeEditor: option(Feature_Editor.Editor.t),
                  ~indentationSettings: IndentationSettings.t,
                  ~statusBar: model,
                  ~theme,
                  ~dispatch,
                  (),
                ) => {
    let%hook activeNotifications =
      CustomHooks.useExpiration(
        ~expireAfter=Feature_Notification.View.Popup.Animations.totalDuration,
        ~equals=(a, b) => Feature_Notification.(a.id == b.id),
        (notifications :> list(Feature_Notification.notification)),
      );

    let (background, foreground) =
      switch (activeNotifications) {
      | [] =>
        Colors.StatusBar.(background.from(theme), foreground.from(theme))
      | [last, ..._] =>
        Feature_Notification.Colors.(
          backgroundFor(last).from(theme),
          foregroundFor(last).from(theme),
        )
      };

    let%hook background =
      CustomHooks.colorTransition(
        ~duration=Feature_Notification.View.Popup.Animations.transitionDuration,
        background,
      );
    let%hook foreground =
      CustomHooks.colorTransition(
        ~duration=Feature_Notification.View.Popup.Animations.transitionDuration,
        foreground,
      );

    let%hook (yOffset, _animationState, _reset) =
      Hooks.animation(transitionAnimation);

    let toStatusBarElement = (statusItem: Item.t) => {
      let onClick =
        statusItem.command
        |> Option.map((command, ()) =>
             dispatch(ContributedItemClicked({id: statusItem.id, command}))
           );

      <item ?onClick>
        <View
          style=Style.[
            flexDirection(`Row),
            justifyContent(`Center),
            alignItems(`Center),
          ]>
          <Label font color=Revery.Colors.white label={statusItem.label} />
        </View>
      </item>;
    };

    let leftItems =
      statusBar.items
      |> List.filter((item: Item.t) => item.alignment == Left)
      |> List.map(toStatusBarElement)
      |> React.listToElement;

    let rightItems =
      statusBar.items
      |> List.filter((item: Item.t) => item.alignment == Right)
      |> List.map(toStatusBarElement)
      |> React.listToElement;

    let indentation = () => {
      let text = indentationSettings |> indentationToString;

      <textItem font background theme text />;
    };

    let fileType = () => {
      let text =
        activeBuffer
        |> OptionEx.flatMap(Buffer.getFileType)
        |> Option.value(~default="plaintext");

      <textItem font background theme text />;
    };

    let lineEndings = () => {
      let toString =
        fun
        | Vim.Types.LF => "LF"
        | Vim.Types.CR => "CR"
        | Vim.Types.CRLF => "CRLF";

      activeBuffer
      |> OptionEx.flatMap(Buffer.getLineEndings)
      |> Option.map(toString)
      |> Option.map(text => {<textItem font background theme text />})
      |> Option.value(~default=React.empty);
    };

    let position = () => {
      let text = {
        OptionEx.map2(
          (editor, buffer) => {
            Feature_Editor.Editor.getPrimaryCursor(~buffer, editor)
          },
          activeEditor,
          activeBuffer,
        )
        |> positionToString;
      };

      <textItem font background theme text />;
    };

    let notificationPopups = () =>
      activeNotifications
      |> List.rev
      |> List.map(model =>
           <Feature_Notification.View.Popup model background foreground font />
         )
      |> React.listToElement;

    <View style={Styles.view(background, yOffset)}>
      <section align=`FlexStart>
        <notificationCount
          dispatch
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
          <diagnosticCount font background theme diagnostics dispatch />
        </section>
        <section align=`Center />
        <section align=`FlexEnd> rightItems </section>
        <section align=`FlexEnd>
          <lineEndings />
          <indentation />
          <fileType />
          <position />
        </section>
        <notificationPopups />
      </sectionGroup>
      <section align=`FlexEnd> <modeIndicator font theme mode /> </section>
    </View>;
  };
};
