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
    color: option(Exthost.Color.t),
    backgroundColor: option(Exthost.Color.t),
    command: option(string),
    tooltip: option(string),
  };

  let create =
      (
        ~color=?,
        ~backgroundColor=?,
        ~command=?,
        ~tooltip=?,
        ~id,
        ~priority,
        ~label,
        ~alignment=Left,
        (),
      ) => {
    id,
    color,
    backgroundColor,
    priority,
    label,
    alignment,
    command,
    tooltip,
  };
};

// MSG

[@deriving show]
type contextMenuMsg =
  | ClearAll
  | Open;

[@deriving show]
type msg =
  | ItemAdded(Item.t)
  | ItemDisposed(string)
  | DiagnosticsClicked
  | FileTypeClicked
  | ContextMenu(Component_ContextMenu.msg(contextMenuMsg))
  | NotificationCountClicked
  | NotificationsCountRightClicked
  | ContributedItemClicked({command: string});

module Msg = {
  let itemAdded = item => ItemAdded(item);
  let itemDisposed = str => ItemDisposed(str);
};

// TODO: Wire these up to Pane / ContextMenu
type outmsg =
  | Nothing
  | ClearNotifications
  | ToggleProblems
  | ToggleNotifications
  | ShowFileTypePicker
  | Effect(Isolinear.Effect.t(msg));

let notificationContextMenu =
  Component_ContextMenu.make([
    Item({
      label: "Clear All",
      // icon: None,
      data: ClearAll,
      details: Revery.UI.React.empty,
    }),
    Item({
      label: "Open",
      // icon: None,
      data: Open,
      details: Revery.UI.React.empty,
    }),
  ]);

type model = {
  items: list(Item.t),
  contextMenu: option(Component_ContextMenu.model(contextMenuMsg)),
};

let initial = {items: [], contextMenu: None};

// UPDATE

let update = (~client, model, msg) => {
  let removeItemById = (items: list(Item.t), id) =>
    List.filter(si => Item.(si.id) != id, items);

  switch (msg) {
  | ItemAdded(item) =>
    /* Replace the old item with the new one */
    let newItems = removeItemById(model.items, item.id);
    ({...model, items: [item, ...newItems]}, Nothing);

  | ItemDisposed(id) => (
      {...model, items: removeItemById(model.items, id)},
      Nothing,
    )

  | NotificationsCountRightClicked => (
      {...model, contextMenu: Some(notificationContextMenu)},
      Nothing,
    )

  | ContextMenu(msg) =>
    model.contextMenu
    |> Option.map(contextMenu => {
         let (contextMenu', outmsg) =
           contextMenu |> Component_ContextMenu.update(msg);

         switch (outmsg) {
         | Component_ContextMenu.Nothing => (
             {...model, contextMenu: Some(contextMenu')},
             Nothing,
           )
         | Component_ContextMenu.Selected({data}) =>
           switch (data) {
           | ClearAll => ({...model, contextMenu: None}, ClearNotifications)
           | Open => ({...model, contextMenu: None}, ToggleNotifications)
           }
         | Component_ContextMenu.Cancelled => (
             {...model, contextMenu: None},
             Nothing,
           )
         };
       })
    |> Option.value(~default=(model, Nothing))

  | FileTypeClicked => (model, ShowFileTypePicker)

  | NotificationCountClicked => (model, ToggleNotifications)

  | DiagnosticsClicked => (model, ToggleProblems)

  | ContributedItemClicked({command, _}) => (
      model,
      Effect(
        Service_Exthost.Effects.Commands.executeContributedCommand(
          ~command,
          ~arguments=[],
          client,
        ),
      ),
    )
  };
};

// VIEW

open EditorCoreTypes;
open Revery;
open Revery.UI;
open Revery.UI.Components;
module Animation = Revery.UI.Animation;
module ContextMenu = Component_ContextMenu;
module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;
module Label = Oni_Components.Label;
module Diagnostics = Feature_Diagnostics;
module Diagnostic = Feature_Diagnostics.Diagnostic;
module Editor = Feature_Editor.Editor;

module Colors = Feature_Theme.Colors;
module Tooltip = Oni_Components.Tooltip;

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
    alignItems(`Center),
    backgroundColor(bg),
    paddingHorizontal(10),
    minWidth(50),
  ];

  let text = (~color) => [
    textWrap(TextWrapping.NoWrap),
    Style.color(color),
  ];
};

let positionToString =
  fun
  | Some(loc: CharacterPosition.t) =>
    Printf.sprintf(
      "%n,%n",
      EditorCoreTypes.LineNumber.toOneBased(loc.line),
      CharacterIndex.toInt(loc.character) + 1,
    )
  | None => "";

let sectionGroup = (~children, ()) =>
  <View style=Styles.sectionGroup> children </View>;

let section = (~children=React.empty, ~align, ()) =>
  <View style={Styles.section(align)}> children </View>;

let item =
    (
      ~key=?,
      ~children,
      ~backgroundColor=Revery.Colors.transparentWhite,
      ~onClick=?,
      ~onRightClick=?,
      (),
    ) => {
  let style = Styles.item(backgroundColor);

  // Avoid cursor turning into pointer if there's no mouse interaction available
  if (onClick == None && onRightClick == None) {
    <View ?key style> children </View>;
  } else {
    <Clickable ?key ?onClick ?onRightClick style> children </Clickable>;
  };
};

let textItem = (~onClick=?, ~font: UiFont.t, ~theme, ~text, ()) =>
  <item ?onClick>
    <Text
      style={Styles.text(~color=Colors.StatusBar.foreground.from(theme))}
      fontFamily={font.family}
      fontSize=11.
      text
    />
  </item>;

let notificationCount =
    (
      ~theme,
      ~font: UiFont.t,
      ~foreground as color,
      ~maybeContextMenu,
      ~notifications: Feature_Notification.model,
      ~dispatch,
      (),
    ) => {
  let text =
    Feature_Notification.all(notifications) |> List.length |> string_of_int;

  let onClick = () => dispatch(NotificationCountClicked);
  let onRightClick = () => dispatch(NotificationsCountRightClicked);

  let menu = () => {
    maybeContextMenu
    |> Option.map(model => {
         <Component_ContextMenu.View
           orientation=(`Top, `Left)
           offsetX=(-10)
           theme
           font
           dispatch={msg => dispatch(ContextMenu(msg))}
           model
         />
       })
    |> Option.value(~default=React.empty);
  };

  <item onClick onRightClick>
    <menu />
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
        style={Styles.text(~color)}
        text
        fontFamily={font.family}
        fontSize=11.
      />
    </View>
  </item>;
};

let diagnosticCount = (~font: UiFont.t, ~theme, ~diagnostics, ~dispatch, ()) => {
  let color = Colors.StatusBar.foreground.from(theme);
  let text = diagnostics |> Feature_Diagnostics.count |> string_of_int;

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
        style={Styles.text(~color)}
        text
        fontFamily={font.family}
        fontSize=11.
      />
    </View>
  </item>;
};

module ModeIndicator = {
  let make = (~key=?, ~font: UiFont.t, ~theme, ~mode, ~subMode, ()) => {
    let background = Colors.Oni.backgroundFor(mode).from(theme);
    let foreground = Colors.Oni.foregroundFor(mode).from(theme);

    let text =
      switch (subMode) {
      | Vim.SubMode.InsertLiteral => "Insert Literal"
      | Vim.SubMode.None => Mode.toString(mode)
      };

    <item ?key backgroundColor=background>
      <Text
        style={Styles.text(~color=foreground)}
        text
        fontFamily={font.family}
        fontWeight=Medium
        fontSize=11.
      />
    </item>;
  };
};

let indentationToString = (indentation: IndentationSettings.t) => {
  switch (indentation.mode) {
  | Tabs => "Tabs: " ++ string_of_int(indentation.tabSize)
  | Spaces => "Spaces: " ++ string_of_int(indentation.size)
  };
};

module View = {
  let make =
      (
        ~key=?,
        ~mode,
        ~subMode,
        ~notifications: Feature_Notification.model,
        ~recordingMacro: option(char),
        ~diagnostics: Diagnostics.model,
        ~font: UiFont.t,
        ~activeBuffer: option(Oni_Core.Buffer.t),
        ~activeEditor: option(Feature_Editor.Editor.t),
        ~indentationSettings: IndentationSettings.t,
        ~scm: Feature_SCM.model,
        ~statusBar: model,
        ~theme,
        ~dispatch,
        ~workingDirectory: string,
        (),
      ) => {
    let activeNotifications = Feature_Notification.active(notifications);
    let background =
      Feature_Notification.statusBarBackground(~theme, notifications);
    let foreground =
      Feature_Notification.statusBarForeground(~theme, notifications);

    let defaultForeground = Colors.StatusBar.foreground.from(theme);

    let yOffset = 0.;

    let toStatusBarElement =
        (~command=?, ~color=?, ~backgroundColor=?, ~tooltip=?, label) => {
      let onClick =
        command
        |> Option.map((command, ()) =>
             dispatch(ContributedItemClicked({command: command}))
           );

      let color =
        color
        |> OptionEx.flatMap(Exthost.Color.resolve(theme))
        |> Option.value(~default=defaultForeground);

      let backgroundColor =
        backgroundColor
        |> OptionEx.flatMap(Exthost.Color.resolve(theme))
        |> Option.value(~default=Revery.Colors.transparentWhite);

      let children = <Label font color label />;
      let style =
        Style.[
          flexDirection(`Row),
          justifyContent(`Center),
          alignItems(`Center),
        ];

      let viewOrTooltip =
        switch (tooltip) {
        | None => <View style> children </View>
        | Some(tooltip) =>
          <Tooltip offsetY=(-25) text=tooltip style> children </Tooltip>
        };

      <item ?onClick backgroundColor> viewOrTooltip </item>;
    };

    let leftItems =
      statusBar.items
      |> List.filter((item: Item.t) => item.alignment == Left)
      |> List.map(
           ({command, label, color, tooltip, backgroundColor, _}: Item.t) =>
           toStatusBarElement(
             ~command?,
             ~backgroundColor?,
             ~color?,
             ~tooltip?,
             label,
           )
         )
      |> React.listToElement;

    let scmItems =
      scm
      |> Feature_SCM.statusBarCommands(~workingDirectory)
      |> List.map((command: Feature_SCM.command) =>
           toStatusBarElement(
             ~command=command.id,
             ~tooltip=?command.tooltip,
             command.title |> Exthost.Label.ofString,
           )
         )
      |> React.listToElement;

    let rightItems =
      statusBar.items
      |> List.filter((item: Item.t) => item.alignment == Right)
      |> List.map(({command, label, color, tooltip, _}: Item.t) =>
           toStatusBarElement(~command?, ~color?, ~tooltip?, label)
         )
      |> React.listToElement;

    let indentation = () => {
      let text = indentationSettings |> indentationToString;

      <textItem font theme text />;
    };

    let fileType = () => {
      let text =
        activeBuffer
        |> Option.map(Buffer.getFileType)
        |> Option.map(Oni_Core.Buffer.FileType.toString)
        |> Option.value(~default=Oni_Core.Buffer.FileType.default);

      <textItem
        font
        theme
        text
        onClick={() => {dispatch(FileTypeClicked)}}
      />;
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
      |> Option.map(text => {<textItem font theme text />})
      |> Option.value(~default=React.empty);
    };

    let position = () => {
      let text = {
        activeEditor
        |> Option.map(Feature_Editor.Editor.getPrimaryCursor)
        |> positionToString;
      };

      <textItem font theme text />;
    };

    let macro = (~register, ()) => {
      <item>
        <View
          style=Style.[
            flexDirection(`Row),
            justifyContent(`Center),
            alignItems(`Center),
          ]>
          <View style=Style.[margin(4)]>
            <Codicon icon=Codicon.circleFilled color=Revery.Colors.red />
          </View>
          <Text
            text={String.make(1, register)}
            style={Styles.text(
              ~color=Colors.StatusBar.foreground.from(theme),
            )}
            fontFamily={font.family}
            fontWeight=Revery.Font.Weight.Bold
            fontSize=11.
          />
        </View>
      </item>;
    };

    let notificationPopups = () =>
      activeNotifications
      |> List.rev
      |> List.map(model =>
           <Feature_Notification.View.Popup model background foreground font />
         )
      |> React.listToElement;

    let macroElement =
      recordingMacro
      |> Option.map(register => <macro register />)
      |> Option.value(~default=React.empty);

    <View ?key style={Styles.view(background, yOffset)}>
      <section align=`FlexStart>
        <notificationCount
          dispatch
          theme
          font
          foreground
          notifications
          maybeContextMenu={statusBar.contextMenu}
        />
      </section>
      <sectionGroup>
        <section align=`FlexStart> macroElement </section>
        <section align=`FlexStart> leftItems </section>
        <section align=`FlexStart>
          <diagnosticCount font theme diagnostics dispatch />
          scmItems
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
      <section align=`FlexEnd>
        <ModeIndicator font theme mode subMode />
      </section>
    </View>;
  };
};

module Configuration = {
  open Config.Schema;
  let visible = setting("workbench.statusBar.visible", bool, ~default=true);
};

module Contributions = {
  let configuration = Configuration.[visible.spec];
};
