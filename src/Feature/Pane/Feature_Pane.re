/*
 * Feature_Pane.re
 */

open Oni_Core;

module Schema = Schema;

module Constants = {
  let defaultHeight = 225;
  let minHeight = 80;
  let maxHeight = 600;
};

[@deriving show({with_path: false})]
type command =
  | ClosePane;

[@deriving show({with_path: false})]
type msg('inner) =
  | PaneClicked
  | TabClicked({index: int})
  | CloseButtonClicked
  | ToggleMaximizeButtonClicked
  | NestedMsg([@opaque] 'inner)
  | Command(command)
  | ResizeHandleDragged(int)
  | ResizeCommitted
  | KeyPressed(string)
  | VimWindowNav(Component_VimWindows.msg)
  | CloseRequested({paneId: string})
  | Toggle({paneId: string});

module Msg = {
  let keyPressed = key => KeyPressed(key);
  let resizeHandleDragged = v => ResizeHandleDragged(v);
  let resizeCommitted = ResizeCommitted;

  let toggle = (~paneId: string) => Toggle({paneId: paneId});

  let close = (~paneId: string) => CloseRequested({paneId: paneId});
};

type outmsg('msg) =
  | Nothing
  | NestedMessage('msg)
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | GrabFocus
  | ReleaseFocus;

type model('model, 'msg) = {
  panes: list(Schema.t('model, 'msg)),
  selected: int,
  allowAnimation: bool,
  isOpen: bool,
  height: int,
  resizeDelta: int,
  vimWindowNavigation: Component_VimWindows.model,
  isMaximized: bool,
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

let indexOfPane = (~paneId: string, model: model('model, 'msg)) => {
  let (maybeFound, _) =
    model.panes
    |> List.fold_left(
         (acc, curr: Schema.t('model, 'msg)) => {
           let (maybeFound, idx) = acc;
           switch (maybeFound) {
           | Some(_) as found => (found, idx)
           | None =>
             if (curr.id == Some(paneId)) {
               (Some(idx), idx);
             } else {
               (None, idx + 1);
             }
           };
         },
         (None, 0),
       );
  maybeFound;
};

let show = (~paneId, model) => {
  let maybeSelected = indexOfPane(~paneId, model);

  maybeSelected
  |> Option.map(selected => {
       {...model, allowAnimation: true, isOpen: true, selected}
     })
  |> Option.value(~default=model);
};
let close = model => {...model, allowAnimation: false, isOpen: false};

let toggle = (~paneId: string, model) => {
  let maybeSelected = indexOfPane(~paneId, model);
  switch (maybeSelected) {
  | None => (close(model), ReleaseFocus)
  | Some(selected) =>
    if (!model.isOpen) {
      (show(~paneId, model), GrabFocus);
    } else if (model.selected == selected) {
      (close(model), ReleaseFocus);
    } else {
      (show(~paneId, model), Nothing);
    }
  };
};

module Focus = {
  let cycleForward = model => {
    let hi = List.length(model.panes) - 1;
    let selected = Utility.IntEx.wrap(~lo=0, ~hi, model.selected + 1);
    {...model, selected};
  };

  let cycleBackward = model => {
    let hi = List.length(model.panes) - 1;
    let selected = Utility.IntEx.wrap(~lo=0, ~hi, model.selected - 1);
    {...model, selected};
  };
};

let activePane = ({selected, panes, _}) => {
  List.nth_opt(panes, selected);
};

let update = (msg, model) =>
  switch (msg) {
  | Command(ClosePane)
  | CloseButtonClicked => ({...model, isOpen: false}, ReleaseFocus)

  | ToggleMaximizeButtonClicked => (
      {...model, isMaximized: !model.isMaximized},
      Nothing,
    )

  | PaneClicked => (model, GrabFocus)

  | TabClicked({index}) => ({...model, selected: index}, Nothing)

  | Toggle({paneId}) => toggle(~paneId, model)

  | CloseRequested({paneId}) =>
    model
    |> activePane
    |> Utility.OptionEx.flatMap((pane: Schema.t('model, 'msg)) =>
         if (pane.id == Some(paneId) && model.isOpen) {
           Some(toggle(~paneId, model));
         } else {
           None;
         }
       )
    |> Option.value(~default=(model, Nothing))

  | ResizeHandleDragged(delta) => (
      {...model, allowAnimation: false, resizeDelta: (-1) * delta},
      Nothing,
    )
  | ResizeCommitted =>
    let height = model |> height;

    if (height <= 0) {
      ({...model, isOpen: false, resizeDelta: 0}, Nothing);
    } else {
      ({...model, height, resizeDelta: 0}, Nothing);
    };

  | KeyPressed(key) =>
    let outmsg =
      model
      |> activePane
      |> Option.map((pane: Schema.t('model, 'msg)) => {
           NestedMessage(pane.keyPressed(key))
         })
      |> Option.value(~default=Nothing);
    (model, outmsg);

  | NestedMsg(msg) => (model, NestedMessage(msg))

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
    | NextTab => (model' |> Focus.cycleForward, Nothing)
    | PreviousTab => (model' |> Focus.cycleBackward, Nothing)
    };
  };

let initial = panes => {
  height: Constants.defaultHeight,
  resizeDelta: 0,
  allowAnimation: true,
  selected: 0,
  isOpen: false,
  panes,
  isMaximized: false,

  vimWindowNavigation: Component_VimWindows.initial,
};

let isOpen = ({isOpen, _}) => isOpen;

let close = model => {...model, isOpen: false};

module View = {
  open Revery.UI;
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
        <Sneakable sneakId={"Pane:" ++ title} onClick style=Styles.clickable>
          <Text
            style={Styles.text(~isActive, ~theme)}
            fontFamily={uiFont.family}
            fontWeight={isActive ? Medium : Normal}
            fontSize={uiFont.size}
            text=title
          />
        </Sneakable>
      </View>;
    };
  };

  module Styles = {
    open Style;

    let pane = (~isMaximized, ~opacity, ~isFocused, ~theme, ~height) => {
      let common = [
        Style.opacity(opacity),
        flexDirection(`Column),
        borderTop(
          ~color=
            isFocused
              ? Colors.focusBorder.from(theme)
              : Colors.Panel.border.from(theme),
          ~width=1,
        ),
        backgroundColor(Colors.Panel.background.from(theme)),
      ];

      if (isMaximized) {
        common
        @ [position(`Absolute), top(0), bottom(0), left(0), right(0)];
      } else if (isFocused) {
        [
          boxShadow(
            ~xOffset=0.,
            ~yOffset=-4.,
            ~blurRadius=isFocused ? 8. : 0.,
            ~spreadRadius=0.,
            ~color=Revery.Color.rgba(0., 0., 0., 0.5),
          ),
          Style.height(height),
          ...common,
        ];
      } else {
        [Style.height(height), ...common];
      };
    };

    let header = [flexDirection(`Row), justifyContent(`SpaceBetween)];

    let buttons = [flexDirection(`Row), justifyContent(`FlexEnd)];

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
        ~activePane,
        ~config,
        ~isFocused,
        ~theme,
        ~iconTheme,
        ~languageInfo,
        ~editorFont,
        ~uiFont,
        ~dispatch,
        ~model,
        ~workingDirectory,
        (),
      ) => {
    switch (activePane) {
    | None => React.empty
    | Some(paneSchema) =>
      Schema.(
        {
          paneSchema.view(
            ~config,
            ~editorFont,
            ~font=uiFont,
            ~isFocused,
            ~iconTheme,
            ~languageInfo,
            ~workingDirectory,
            ~theme,
            ~model,
            ~dispatch=msg =>
            dispatch(NestedMsg(msg))
          );
        }
      )
    };
  };

  // TODO: Workaround not having a 'stopPropagation' or 'preventDefault' gesture...
  let wasCloseButtonClicked = ref(false);

  let closeButton = (~theme, ~dispatch, ()) => {
    <Sneakable
      sneakId="close"
      onClick={() => {
        wasCloseButtonClicked := true;
        dispatch(CloseButtonClicked);
      }}
      style=Styles.closeButton>
      <Codicon
        icon=Codicon.close
        color={Colors.Tab.activeForeground.from(theme)}
      />
    </Sneakable>;
  };

  let toggleMaximizeButton = (~isMaximized: bool, ~theme, ~dispatch, ()) => {
    <Sneakable
      sneakId="toggle"
      onClick={() => dispatch(ToggleMaximizeButtonClicked)}
      style=Styles.closeButton>
      <Codicon
        icon={isMaximized ? Codicon.chevronDown : Codicon.chevronUp}
        color={Colors.Tab.activeForeground.from(theme)}
      />
    </Sneakable>;
  };

  let make =
      (
        ~config,
        ~isFocused,
        ~theme,
        ~iconTheme,
        ~languageInfo,
        ~editorFont: Service_Font.font,
        ~uiFont,
        ~dispatch: msg('msg) => unit,
        ~pane: model('model, 'msg),
        ~model: 'model,
        ~workingDirectory: string,
        (),
      ) => {
    let desiredHeight = height(pane);
    let height = !isOpen(pane) && !isFocused ? 0 : desiredHeight;

    let selected = pane.selected;
    let paneTabs =
      pane.panes
      |> List.mapi((idx, schema) => {
           Schema.(
             <PaneTab
               uiFont
               theme
               title={schema.title}
               onClick={_ => dispatch(TabClicked({index: idx}))}
               isActive={selected == idx}
             />
           )
         })
      |> React.listToElement;

    let activePane = activePane(pane);

    let customPaneButtons =
      activePane
      |> Option.map((pane: Schema.t('model, 'msg)) =>
           pane.buttons(
             ~font=uiFont,
             ~theme,
             ~dispatch=msg => dispatch(NestedMsg(msg)),
             ~model,
           )
         )
      |> Option.value(~default=React.empty);

    let opacity =
      isFocused
        ? 1.0
        : Feature_Configuration.GlobalConfiguration.inactiveWindowOpacity.get(
            config,
          );
    height == 0
      ? React.empty
      : <View
          style={Styles.pane(
            ~isMaximized=pane.isMaximized,
            ~opacity,
            ~isFocused,
            ~theme,
            ~height,
          )}
          onMouseUp={_ => {
            if (! wasCloseButtonClicked^) {
              dispatch(PaneClicked);
            };
            wasCloseButtonClicked := false;
          }}>
          <View style=Styles.resizer>
            <ResizeHandle.Horizontal
              onDrag={delta =>
                dispatch(Msg.resizeHandleDragged(int_of_float(delta)))
              }
              onDragComplete={() => dispatch(Msg.resizeCommitted)}
            />
          </View>
          <View style=Styles.header>
            <View style=Styles.tabs> paneTabs </View>
            <View style=Styles.buttons>
              customPaneButtons
              <toggleMaximizeButton
                isMaximized={pane.isMaximized}
                dispatch
                theme
              />
              <closeButton dispatch theme />
            </View>
          </View>
          <View style=Styles.content>
            <content
              config
              isFocused
              iconTheme
              languageInfo
              theme
              dispatch
              uiFont
              editorFont
              activePane
              model
              workingDirectory
            />
          </View>
        </View>;
  };
};

module Commands = {
  open Feature_Commands.Schema;

  let closePane =
    define(
      // TODO: Is there a VSCode equivalent?
      "workbench.actions.pane.close",
      Command(ClosePane),
    );
};

module Keybindings = {
  open Feature_Input.Schema;
  let escKey =
    bind(
      ~key="<ESC>",
      ~command=Commands.closePane.id,
      ~condition="paneFocus" |> WhenExpr.parse,
    );
};

module Contributions = {
  let commands = (~isFocused, model: 'model, pane: model('model, 'msg)) => {
    let common = Commands.[closePane];
    let vimWindowCommands =
      Component_VimWindows.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => VimWindowNav(msg)));

    let activePanelCommands =
      pane
      |> activePane
      |> Option.map((p: Schema.t('model, 'msg)) => {p.commands(model)})
      |> Option.value(~default=[])
      |> List.map(Oni_Core.Command.map(msg => NestedMsg(msg)));

    isFocused ? common @ vimWindowCommands @ activePanelCommands : common;
  };

  let contextKeys = (~isFocused, model: 'model, pane: model('model, 'msg)) => {
    module PanelSchema = Schema;
    open WhenExpr.ContextKeys;
    let vimNavKeys =
      isFocused
        ? Component_VimWindows.Contributions.contextKeys(
            pane.vimWindowNavigation,
          )
        : empty;

    let activePanel =
      pane
      |> activePane
      |> Utility.OptionEx.flatMap((pane: PanelSchema.t('model, 'msg)) => {
           pane.id
         })
      |> Option.map(id => {[Schema.string("activePanel", _ => id)]})
      |> Option.value(~default=[])
      |> Schema.fromList
      |> fromSchema(pane);

    let activePanelContextKeys =
      pane
      |> activePane
      |> Option.map((pane: PanelSchema.t('model, 'msg)) => {
           pane.contextKeys(~isFocused, model)
         })
      |> Option.value(~default=empty);

    let paneFocus =
      [Schema.bool("paneFocus", _ => isFocused)]
      |> Schema.fromList
      |> fromSchema(pane);

    [activePanel, activePanelContextKeys, paneFocus, vimNavKeys] |> unionMany;
  };

  let keybindings = Keybindings.[escKey];
};

let sub = (~isFocused, outerModel, pane) => {
  pane
  |> activePane
  |> Option.map((pane: Schema.t('model, 'msg)) => {
       pane.sub(~isFocused, outerModel)
       |> Isolinear.Sub.map(msg => NestedMsg(msg))
     })
  |> Option.value(~default=Isolinear.Sub.none);
};
