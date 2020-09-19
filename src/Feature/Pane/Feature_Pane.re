/*
 * Feature_Pane.re
 */

open Oni_Core;

[@deriving show({with_path: false})]
type pane =
  | Diagnostics
  | Notifications;

module Constants = {
  let defaultHeight = 225;
  let minHeight = 80;
  let maxHeight = 600;
};

[@deriving show({with_path: false})]
type command =
  | ToggleProblems;

[@deriving show({with_path: false})]
type msg =
  | TabClicked(pane)
  | CloseButtonClicked
  | DiagnosticItemClicked({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | Command(command)
  | ResizeHandleDragged(int)
  | ResizeCommitted
  | KeyPressed(string)
  | VimWindowNav(Component_VimWindows.msg);

module Msg = {
  let keyPressed = key => KeyPressed(key);
  let resizeHandleDragged = v => ResizeHandleDragged(v);
  let resizeCommitted = ResizeCommitted;
};

type outmsg =
  | Nothing
  | OpenFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | UnhandledWindowMovement(Component_VimWindows.outmsg);

type model = {
  selected: pane,
  isOpen: bool,
  height: int,
  resizeDelta: int,
  vimWindowNavigation: Component_VimWindows.model,
};

let height = ({height, resizeDelta, _}) => {
  let candidateHeight = height + resizeDelta;
  if (candidateHeight < Constants.minHeight) {
    0;
  } else if (candidateHeight > Constants.maxHeight) {
    Constants.maxHeight;
  } else {
    candidateHeight;
  };
};

let show = (~pane, model) => {...model, isOpen: true, selected: pane};
let close = model => {...model, isOpen: false};

module Focus = {
  let toggleTab = model => {
    let pane =
      switch (model.selected) {
      | Diagnostics => Notifications
      | Notifications => Diagnostics
      };
    {...model, selected: pane};
  };
};

let update = (msg, model) =>
  switch (msg) {
  | DiagnosticItemClicked({filePath, position}) => (
      model,
      OpenFile({filePath, position}),
    )
  | CloseButtonClicked => ({...model, isOpen: false}, Nothing)

  | TabClicked(pane) => ({...model, selected: pane}, Nothing)

  | Command(ToggleProblems) =>
    if (!model.isOpen) {
      (show(~pane=Diagnostics, model), Nothing);
    } else if (model.selected == Diagnostics) {
      (close(model), Nothing);
    } else {
      (show(~pane=Diagnostics, model), Nothing);
    }

  | ResizeHandleDragged(delta) => (
      {...model, resizeDelta: (-1) * delta},
      Nothing,
    )
  | ResizeCommitted =>
    let height = model |> height;

    if (height <= 0) {
      ({...model, isOpen: false, resizeDelta: 0}, Nothing);
    } else {
      ({...model, height, resizeDelta: 0}, Nothing);
    };

  | KeyPressed(_) => (model, Nothing)

  | VimWindowNav(navMsg) =>
    let (vimWindowNavigation, outmsg) =
      Component_VimWindows.update(navMsg, model.vimWindowNavigation);

    let model' = {...model, vimWindowNavigation};

    switch (outmsg) {
    | Nothing => (model', Nothing)
    | FocusLeft
    | FocusRight
    | FocusDown
    | FocusUp => (model', UnhandledWindowMovement(outmsg))
    | NextTab => (model' |> Focus.toggleTab, Nothing)
    | PreviousTab => (model' |> Focus.toggleTab, Nothing)
    };
  };

let initial = {
  height: Constants.defaultHeight,
  resizeDelta: 0,
  selected: Notifications,
  isOpen: false,

  vimWindowNavigation: Component_VimWindows.initial,
};

let selected = ({selected, _}) => selected;

let isVisible = (pane, model) => model.isOpen && model.selected == pane;
let isOpen = ({isOpen, _}) => isOpen;

let toggle = (~pane, model) =>
  if (model.isOpen && model.selected == pane) {
    {...model, isOpen: false};
  } else {
    {...model, isOpen: true, selected: pane};
  };

let close = model => {...model, isOpen: false};

module View = {
  open Revery.UI;
  open Revery.UI.Components;
  open Oni_Components;

  module FontIcon = Oni_Components.FontIcon;
  module FontAwesome = Oni_Components.FontAwesome;
  module Sneakable = Feature_Sneak.View.Sneakable;

  module Colors = Feature_Theme.Colors;
  module PaneTab = {
    module Constants = {
      let minWidth = 100;
    };

    module Styles = {
      open Style;

      let container = (~isActive, ~theme) => {
        let borderColor =
          isActive ? Colors.PanelTitle.activeBorder : Colors.Panel.background;

        [
          overflow(`Hidden),
          paddingHorizontal(5),
          backgroundColor(Colors.Panel.background.from(theme)),
          borderBottom(~color=borderColor.from(theme), ~width=2),
          height(30),
          minWidth(Constants.minWidth),
          flexDirection(`Row),
          justifyContent(`Center),
          alignItems(`Center),
        ];
      };

      let clickable = [
        flexGrow(1),
        flexDirection(`Row),
        alignItems(`Center),
        justifyContent(`Center),
      ];

      let text = (~isActive, ~theme) => [
        textOverflow(`Ellipsis),
        isActive
          ? color(Colors.PanelTitle.activeForeground.from(theme))
          : color(Colors.PanelTitle.inactiveForeground.from(theme)),
        justifyContent(`Center),
        alignItems(`Center),
      ];
    };

    let make = (~uiFont: UiFont.t, ~theme, ~title, ~onClick, ~isActive, ()) => {
      <View style={Styles.container(~isActive, ~theme)}>
        <Clickable onClick style=Styles.clickable>
          <Text
            style={Styles.text(~isActive, ~theme)}
            fontFamily={uiFont.family}
            fontWeight={isActive ? Medium : Normal}
            fontSize={uiFont.size}
            text=title
          />
        </Clickable>
      </View>;
    };
  };

  module Styles = {
    open Style;

    let pane = (~isFocused, ~theme, ~height) => {
      let common = [
        flexDirection(`Column),
        Style.height(height),
        borderTop(
          ~color=
            isFocused
              ? Colors.focusBorder.from(theme)
              : Colors.Panel.border.from(theme),
          ~width=1,
        ),
        backgroundColor(Colors.Panel.background.from(theme)),
      ];

      if (isFocused) {
        [
          boxShadow(
            ~xOffset=0.,
            ~yOffset=-4.,
            ~blurRadius=isFocused ? 8. : 0.,
            ~spreadRadius=0.,
            ~color=Revery.Color.rgba(0., 0., 0., 0.5),
          ),
          ...common,
        ];
      } else {
        common;
      };
    };

    let header = [flexDirection(`Row), justifyContent(`SpaceBetween)];

    let tabs = [flexDirection(`Row)];

    let closeButton = [
      width(32),
      alignItems(`Center),
      justifyContent(`Center),
    ];

    let resizer = [height(4), position(`Relative), flexGrow(0)];

    let content = [flexDirection(`Column), flexGrow(1)];
  };
  let content =
      (
        ~selected,
        ~theme,
        ~uiFont,
        ~editorFont,
        ~onSelectFile,
        ~notificationDispatch,
        ~diagnostics: Feature_LanguageSupport.Diagnostics.t,
        ~notifications: Feature_Notification.model,
        (),
      ) =>
    switch (selected) {
    | Diagnostics =>
      <Feature_LanguageSupport.Diagnostics.View
        diagnostics
        theme
        uiFont
        editorFont
        onSelectFile
      />
    | Notifications =>
      <Feature_Notification.View.List
        model=notifications
        theme
        font=uiFont
        dispatch=notificationDispatch
      />
    };

  let closeButton = (~theme, ~dispatch, ()) => {
    <Sneakable
      sneakId="close"
      onClick={() => dispatch(CloseButtonClicked)}
      style=Styles.closeButton>
      <FontIcon
        icon=FontAwesome.times
        color={Colors.Tab.activeForeground.from(theme)}
        fontSize=12.
      />
    </Sneakable>;
  };
  let make =
      (
        ~isFocused,
        ~theme,
        ~uiFont,
        ~editorFont,
        ~diagnostics: Feature_LanguageSupport.Diagnostics.t,
        ~notifications: Feature_Notification.model,
        ~dispatch: msg => unit,
        ~notificationDispatch: Feature_Notification.msg => unit,
        ~pane: model,
        (),
      ) => {
    let onSelectFile = (~filePath, ~position) => {
      dispatch(DiagnosticItemClicked({filePath, position}));
    };

    let problemsTabClicked = () => {
      dispatch(TabClicked(Diagnostics));
    };
    let notificationsTabClicked = () => {
      dispatch(TabClicked(Notifications));
    };

    if (!isOpen(pane)) {
      <View />;
    } else {
      let height = height(pane);
      <View style={Styles.pane(~isFocused, ~theme, ~height)}>
        <View style=Styles.resizer>
          <ResizeHandle.Horizontal
            onDrag={delta =>
              dispatch(Msg.resizeHandleDragged(int_of_float(delta)))
            }
            onDragComplete={() => dispatch(Msg.resizeCommitted)}
          />
        </View>
        <View style=Styles.header>
          <View style=Styles.tabs>
            <PaneTab
              uiFont
              theme
              title="Problems"
              onClick=problemsTabClicked
              isActive={isVisible(Diagnostics, pane)}
            />
            <PaneTab
              uiFont
              theme
              title="Notifications"
              onClick=notificationsTabClicked
              isActive={isVisible(Notifications, pane)}
            />
          </View>
          <closeButton dispatch theme />
        </View>
        <View style=Styles.content>
          <content
            selected={selected(pane)}
            theme
            uiFont
            editorFont
            diagnostics
            notifications
            notificationDispatch
            onSelectFile
          />
        </View>
      </View>;
    };
  };
};

module Commands = {
  open Feature_Commands.Schema;
  let problems =
    define(
      ~category="View",
      ~title="Toggle Problems (Errors, Warnings)",
      "workbench.actions.view.problems",
      Command(ToggleProblems),
    );
};

module Keybindings = {
  open Oni_Input.Keybindings;
  let toggleProblems = {
    key: "<S-C-M>",
    command: Commands.problems.id,
    condition: WhenExpr.Value(True),
  };

  let toggleProblemsOSX = {
    key: "<D-S-M>",
    command: Commands.problems.id,
    condition: "isMac" |> WhenExpr.parse,
  };
};

module Contributions = {
  let commands = (~isFocused) => {
    let common = Commands.[problems];
    let vimWindowCommands =
      Component_VimWindows.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => VimWindowNav(msg)));
    isFocused ? common @ vimWindowCommands : common;
  };

  open WhenExpr.ContextKeys.Schema;
  let contextKeys = (~isFocused) => {
    let vimNavKeys =
      isFocused ? Component_VimWindows.Contributions.contextKeys : [];

    [
      vimNavKeys
      |> fromList
      |> map(({vimWindowNavigation, _}: model) => vimWindowNavigation),
    ]
    |> unionMany;
  };

  let keybindings = Keybindings.[toggleProblems, toggleProblemsOSX];
};
