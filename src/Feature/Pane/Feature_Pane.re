/*
 * Feature_Pane.re
 */

open EditorCoreTypes;
open Oni_Core;

[@deriving show({with_path: false})]
type pane =
  | Diagnostics
  | Notifications
  | Locations
  | Output;

module Constants = {
  let defaultHeight = 225;
  let minHeight = 80;
  let maxHeight = 600;
};

[@deriving show({with_path: false})]
type command =
  | ToggleProblems
  | ToggleMessages
  | ClosePane;

[@deriving show({with_path: false})]
type msg =
  | TabClicked(pane)
  | CloseButtonClicked
  | PaneButtonClicked(pane)
  | Command(command)
  | ResizeHandleDragged(int)
  | ResizeCommitted
  | KeyPressed(string)
  | VimWindowNav(Component_VimWindows.msg)
  | DiagnosticsList(Component_VimTree.msg)
  | LocationsList(Component_VimTree.msg)
  | NotificationsList(Component_VimList.msg)
  | OutputPane(Component_Output.msg)
  | DismissNotificationClicked(Feature_Notification.notification)
  | LocationFileLoaded({
      filePath: string,
      lines: array(string),
    });

module Msg = {
  let keyPressed = key => KeyPressed(key);
  let resizeHandleDragged = v => ResizeHandleDragged(v);
  let resizeCommitted = ResizeCommitted;

  let toggleMessages = Command(ToggleMessages);
};

module Effects = {
  let expandLocationPath = (~font, ~languageInfo, ~buffers, ~filePath) => {
    let toMsg = lines => LocationFileLoaded({filePath, lines});
    Feature_Buffers.Effects.loadFile(
      ~font,
      ~languageInfo,
      ~filePath,
      ~toMsg,
      buffers,
    );
  };
};

type outmsg =
  | Nothing
  | PaneButton(pane)
  | OpenFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | PreviewFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | GrabFocus
  | ReleaseFocus
  | NotificationDismissed(Feature_Notification.notification)
  | Effect(Isolinear.Effect.t(msg));

type model = {
  selected: pane,
  allowAnimation: bool,
  isOpen: bool,
  height: int,
  resizeDelta: int,
  vimWindowNavigation: Component_VimWindows.model,
  notificationsView:
    Component_VimList.model(Feature_Notification.notification),
  diagnosticsView:
    Component_VimTree.model(string, Oni_Components.LocationListItem.t),
  locationNodes:
    list(
      Tree.t(LocationsPaneView.location, Oni_Components.LocationListItem.t),
    ),
  locationsView:
    Component_VimTree.model(
      LocationsPaneView.location,
      Oni_Components.LocationListItem.t,
    ),
  outputPane: option(Component_Output.model),
};

let locationsToReferences = (locations: list(Exthost.Location.t)) => {
  let map =
    List.fold_left(
      (acc, curr: Exthost.Location.t) => {
        let {range, uri}: Exthost.Location.t = curr;

        let range = range |> Exthost.OneBasedRange.toRange;

        let path = Oni_Core.Uri.toFileSystemPath(uri);

        acc
        |> StringMap.update(
             path,
             fun
             | None => Some([range])
             | Some(ranges) => Some([range, ...ranges]),
           );
      },
      StringMap.empty,
      locations,
    );

  map
  |> StringMap.bindings
  |> List.map(((path, ranges)) => LocationsPaneView.{path, ranges});
};

let updateLocationTree = (nodes, model) => {
  let locationsView' =
    Component_VimTree.set(
      ~uniqueId=(LocationsPaneView.{path, _}) => path,
      ~searchText=
        Component_VimTree.(
          fun
          | Node({data, _}) => LocationsPaneView.(data.path)
          | Leaf({data, _}) => Oni_Components.LocationListItem.(data.text)
        ),
      nodes,
      model.locationsView,
    );

  {...model, locationNodes: nodes, locationsView: locationsView'};
};

let expandLocation = (~filePath, ~lines, model) => {
  let characterIndexToIndex = (idx: CharacterIndex.t) => {
    idx |> CharacterIndex.toInt |> Index.fromZeroBased;
  };

  let expandChildren = (location: LocationsPaneView.location) => {
    let lineCount = Array.length(lines);
    location.ranges
    |> List.filter_map((range: CharacterRange.t) => {
         let line = range.start.line |> EditorCoreTypes.LineNumber.toZeroBased;
         if (line >= 0 && line < lineCount) {
           let highlight =
             if (range.start.line == range.stop.line) {
               Some((
                 range.start.character |> characterIndexToIndex,
                 range.stop.character |> characterIndexToIndex,
               ));
             } else {
               None;
             };
           Some(
             Oni_Components.LocationListItem.{
               file: filePath,
               location: range.start,
               text: lines[line],
               highlight,
             },
           );
         } else {
           None;
         };
       })
    |> List.sort(
         (
           a: Oni_Components.LocationListItem.t,
           b: Oni_Components.LocationListItem.t,
         ) => {
         (a.location.line |> EditorCoreTypes.LineNumber.toZeroBased)
         - (b.location.line |> EditorCoreTypes.LineNumber.toZeroBased)
       });
  };

  let locationNodes' =
    model.locationNodes
    |> List.map(
         fun
         | Tree.Leaf(_) as leaf => leaf
         | Tree.Node({data, _} as prev) =>
           if (LocationsPaneView.(data.path) == filePath) {
             let children = data |> expandChildren |> List.map(Tree.leaf);

             Node({expanded: true, data, children});
           } else {
             Node(prev);
           },
       );

  model |> updateLocationTree(locationNodes');
};

let collapseLocations = model => {
  ...model,
  locationsView: Component_VimTree.collapse(model.locationsView),
};

let setLocations = (~maybeActiveBuffer, ~locations, model) => {
  let references = locationsToReferences(locations);

  let nodes =
    references
    |> List.map(reference =>
         Tree.node(~children=[], ~expanded=false, reference)
       );

  let model' =
    model
    // Un-expand all the nodes
    |> collapseLocations
    |> updateLocationTree(nodes);

  // Try to expand the current buffer, if it's available
  maybeActiveBuffer
  |> Utility.OptionEx.flatMap(buffer => {
       switch (Buffer.getFilePath(buffer)) {
       | None => None
       | Some(filePath) =>
         let lines = Buffer.getLines(buffer);
         Some(model' |> expandLocation(~filePath, ~lines));
       }
     })
  |> Option.value(~default=model');
};
let diagnosticToLocList =
    (diagWithUri: (Uri.t, Feature_Diagnostics.Diagnostic.t)) => {
  let (uri, diag) = diagWithUri;
  let file = Uri.toFileSystemPath(uri);
  let location = Feature_Diagnostics.Diagnostic.(diag.range.start);
  Oni_Components.LocationListItem.{
    file,
    location,
    text: diag.message,
    highlight: None,
  };
};

let setNotifications = (notifications, model) => {
  let searchText = (notification: Feature_Notification.notification) => {
    notification.message;
  };
  let notificationsArray =
    notifications |> Feature_Notification.all |> Array.of_list;

  let notificationsView' =
    model.notificationsView
    |> Component_VimList.set(~searchText, notificationsArray);
  {...model, notificationsView: notificationsView'};
};

let setDiagnostics = (diagnostics, model) => {
  let diagLocList =
    diagnostics
    |> Feature_Diagnostics.getAllDiagnostics
    |> List.map(diagnosticToLocList)
    |> Oni_Components.LocationListItem.toTrees;

  let diagnosticsView' =
    Component_VimTree.set(
      ~uniqueId=path => path,
      ~searchText=
        Component_VimTree.(
          fun
          | Node({data, _}) => data
          | Leaf({data, _}) => Oni_Components.LocationListItem.(data.text)
        ),
      diagLocList,
      model.diagnosticsView,
    );
  {...model, diagnosticsView: diagnosticsView'};
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

let setPane = (~pane, model) => {...model, selected: pane};

let show = (~pane, model) => {
  ...model,
  allowAnimation: true,
  isOpen: true,
  selected: pane,
};
let close = model => {...model, allowAnimation: false, isOpen: false};

let setOutput = (_cmd, maybeContents, model) => {
  let outputPane' =
    maybeContents
    |> Option.map(output =>
         Component_Output.set(output, Component_Output.initial)
       );
  {...model, outputPane: outputPane'} |> setPane(~pane=Output);
};

module Focus = {
  let cycleForward = model => {
    let pane =
      switch (model.selected) {
      | Diagnostics => Notifications
      | Notifications => Locations
      | Locations => Output
      | Output => Diagnostics
      };
    {...model, selected: pane};
  };

  let cycleBackward = model => {
    let pane =
      switch (model.selected) {
      | Output => Locations
      | Notifications => Diagnostics
      | Locations => Notifications
      | Diagnostics => Output
      };
    {...model, selected: pane};
  };
};

let update = (~buffers, ~font, ~languageInfo, ~previewEnabled, msg, model) =>
  switch (msg) {
  | Command(ClosePane)
  | CloseButtonClicked => ({...model, isOpen: false}, ReleaseFocus)

  | DismissNotificationClicked(notification) => (
      model,
      NotificationDismissed(notification),
    )

  | TabClicked(pane) => ({...model, selected: pane}, Nothing)

  | Command(ToggleProblems) =>
    if (!model.isOpen) {
      (show(~pane=Diagnostics, model), GrabFocus);
    } else if (model.selected == Diagnostics) {
      (close(model), ReleaseFocus);
    } else {
      (show(~pane=Diagnostics, model), Nothing);
    }

  | PaneButtonClicked(pane) => (model, PaneButton(pane))

  | Command(ToggleMessages) =>
    if (!model.isOpen) {
      (show(~pane=Notifications, model), GrabFocus);
    } else if (model.selected == Notifications) {
      (close(model), ReleaseFocus);
    } else {
      (show(~pane=Notifications, model), Nothing);
    }

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
    switch (model.selected) {
    | Notifications => (
        {
          ...model,
          notificationsView:
            Component_VimList.keyPress(key, model.notificationsView),
        },
        Nothing,
      )

    | Diagnostics => (
        {
          ...model,
          diagnosticsView:
            Component_VimTree.keyPress(key, model.diagnosticsView),
        },
        Nothing,
      )
    | Locations => (
        {
          ...model,
          locationsView: Component_VimTree.keyPress(key, model.locationsView),
        },
        Nothing,
      )

    | Output => (
        {
          ...model,
          outputPane:
            model.outputPane |> Option.map(Component_Output.keyPress(key)),
        },
        Nothing,
      )
    }

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

  | LocationsList(listMsg) =>
    let (locationsView, outmsg) =
      Component_VimTree.update(listMsg, model.locationsView);

    let eff =
      switch (outmsg) {
      | Component_VimTree.Nothing => Nothing
      | Component_VimTree.Touched(item) =>
        previewEnabled
          ? PreviewFile({filePath: item.file, position: item.location})
          : OpenFile({filePath: item.file, position: item.location})
      | Component_VimTree.Selected(item) =>
        OpenFile({filePath: item.file, position: item.location})
      | Component_VimTree.Collapsed(_) => Nothing
      | Component_VimTree.Expanded({path, _}) =>
        Effect(
          Effects.expandLocationPath(
            ~font,
            ~languageInfo,
            ~buffers,
            ~filePath=path,
          ),
        )
      };

    ({...model, locationsView}, eff);

  | LocationFileLoaded({filePath, lines}) => (
      model |> expandLocation(~filePath, ~lines),
      Nothing,
    )

  | NotificationsList(listMsg) =>
    let (notificationsView, outmsg) =
      Component_VimList.update(listMsg, model.notificationsView);

    let eff =
      switch (outmsg) {
      | Component_VimList.Nothing => Nothing
      | Component_VimList.Selected(_) => Nothing
      | Component_VimList.Touched(_) => Nothing
      };

    ({...model, notificationsView}, eff);

  | DiagnosticsList(listMsg) =>
    let (diagnosticsView, outmsg) =
      Component_VimTree.update(listMsg, model.diagnosticsView);

    let eff =
      switch (outmsg) {
      | Component_VimTree.Nothing => Nothing
      | Component_VimTree.Touched(item) =>
        previewEnabled
          ? PreviewFile({filePath: item.file, position: item.location})
          : OpenFile({filePath: item.file, position: item.location})
      | Component_VimTree.Selected(item) =>
        OpenFile({filePath: item.file, position: item.location})
      | Component_VimTree.Collapsed(_) => Nothing
      | Component_VimTree.Expanded(_) => Nothing
      };

    ({...model, diagnosticsView}, eff);

  | OutputPane(outputMsg) =>
    model.outputPane
    |> Option.map(outputPane => {
         let (outputPane, outmsg) =
           Component_Output.update(outputMsg, outputPane);

         let model' = {...model, outputPane: Some(outputPane)};
         switch (outmsg) {
         | Nothing => (model', Nothing)
         // Emulate Vim behavior on space / enter - close pane
         | Selected => ({...model', isOpen: false}, ReleaseFocus)
         };
       })
    |> Option.value(~default=(model, Nothing))
  };

let initial = {
  height: Constants.defaultHeight,
  resizeDelta: 0,
  allowAnimation: true,
  selected: Diagnostics,
  isOpen: false,

  vimWindowNavigation: Component_VimWindows.initial,
  diagnosticsView: Component_VimTree.create(~rowHeight=20),

  locationNodes: [],
  locationsView: Component_VimTree.create(~rowHeight=20),
  notificationsView: Component_VimList.create(~rowHeight=20),

  outputPane: None,
};

let selected = ({selected, _}) => selected;

let isSelected = (pane, model) => model.selected == pane;
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

    let pane = (~opacity, ~isFocused, ~theme, ~height) => {
      let common = [
        Style.opacity(opacity),
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

    let buttons = [flexDirection(`Row), justifyContent(`FlexEnd)];

    let tabs = [flexDirection(`Row)];

    let closeButton = [
      width(32),
      alignItems(`Center),
      justifyContent(`Center),
    ];

    let paneButton = [
      width(32),
      alignItems(`Center),
      justifyContent(`Center),
    ];

    let resizer = [height(4), position(`Relative), flexGrow(0)];

    let content = [flexDirection(`Column), flexGrow(1)];
  };
  let content =
      (
        ~isFocused,
        ~selected,
        ~theme,
        ~iconTheme,
        ~languageInfo,
        ~editorFont,
        ~uiFont,
        ~dispatch,
        ~outputPane,
        ~locationsList,
        ~locationsDispatch: Component_VimTree.msg => unit,
        ~diagnosticDispatch: Component_VimTree.msg => unit,
        ~diagnosticsList: Component_VimTree.model(string, LocationListItem.t),
        ~notificationsList:
           Component_VimList.model(Feature_Notification.notification),
        ~notificationsDispatch: Component_VimList.msg => unit,
        ~outputDispatch: Component_Output.msg => unit,
        ~workingDirectory,
        (),
      ) =>
    switch (selected) {
    | Locations =>
      <LocationsPaneView
        isFocused
        locationsList
        iconTheme
        languageInfo
        theme
        uiFont
        workingDirectory
        dispatch=locationsDispatch
      />

    | Diagnostics =>
      <DiagnosticsPaneView
        isFocused
        diagnosticsList
        iconTheme
        languageInfo
        theme
        uiFont
        workingDirectory
        dispatch=diagnosticDispatch
      />
    | Notifications =>
      <NotificationsPaneView
        isFocused
        notificationsList
        theme
        uiFont
        dispatch=notificationsDispatch
        onDismiss={notification =>
          dispatch(DismissNotificationClicked(notification))
        }
      />
    | Output =>
      outputPane
      |> Option.map(model => {
           <Component_Output.View
             model
             isActive=isFocused
             editorFont
             uiFont
             theme
             dispatch=outputDispatch
           />
         })
      |> Option.value(~default=React.empty)
    };

  module Animation = {
    let openSpring = Spring.Options.create(~stiffness=600., ~damping=50., ());
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

  let paneButton = (~theme, ~dispatch, ~pane, ()) =>
    switch (pane) {
    | Notifications =>
      <Sneakable
        sneakId="paneButton"
        onClick={() => dispatch(PaneButtonClicked(pane))}
        style=Styles.paneButton>
        <FontIcon
          icon={
            switch (pane) {
            | Notifications => FontAwesome.bellSlash
            | _ => FontAwesome.cross
            }
          }
          color={Colors.Tab.activeForeground.from(theme)}
          fontSize=12.
        />
      </Sneakable>
    | _ => React.empty
    };
  let%component make =
                (
                  ~config,
                  ~isFocused,
                  ~theme,
                  ~iconTheme,
                  ~languageInfo,
                  ~editorFont: Service_Font.font,
                  ~uiFont,
                  ~dispatch: msg => unit,
                  ~pane: model,
                  ~workingDirectory: string,
                  (),
                ) => {
    let problemsTabClicked = () => {
      dispatch(TabClicked(Diagnostics));
    };
    let notificationsTabClicked = () => {
      dispatch(TabClicked(Notifications));
    };
    let locationsTabClicked = () => {
      dispatch(TabClicked(Locations));
    };
    let outputTabClicked = () => {
      dispatch(TabClicked(Output));
    };

    let desiredHeight = height(pane);
    let targetHeight = !isOpen(pane) && !isFocused ? 0 : desiredHeight;

    let animationEnabled =
      pane.allowAnimation
      && Feature_Configuration.GlobalConfiguration.animation.get(config);
    let%hook (springHeight, _setHeightImmediately) =
      Hooks.spring(
        ~name="Pane Open Spring",
        ~target=float(targetHeight),
        ~restThreshold=10.,
        ~enabled=animationEnabled,
        Animation.openSpring,
      );

    let height = springHeight < 20. ? 0 : int_of_float(springHeight);

    let opacity =
      isFocused
        ? 1.0
        : Feature_Configuration.GlobalConfiguration.inactiveWindowOpacity.get(
            config,
          );
    height == 0
      ? React.empty
      : <View style={Styles.pane(~opacity, ~isFocused, ~theme, ~height)}>
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
                isActive={isSelected(Diagnostics, pane)}
              />
              <PaneTab
                uiFont
                theme
                title="Notifications"
                onClick=notificationsTabClicked
                isActive={isSelected(Notifications, pane)}
              />
              <PaneTab
                uiFont
                theme
                title="Locations"
                onClick=locationsTabClicked
                isActive={isSelected(Locations, pane)}
              />
              <PaneTab
                uiFont
                theme
                title="Output"
                onClick=outputTabClicked
                isActive={isSelected(Output, pane)}
              />
            </View>
            <View style=Styles.buttons>
              <paneButton dispatch theme pane={pane.selected} />
              <closeButton dispatch theme />
            </View>
          </View>
          <View style=Styles.content>
            <content
              isFocused
              iconTheme
              languageInfo
              diagnosticsList={pane.diagnosticsView}
              locationsList={pane.locationsView}
              notificationsList={pane.notificationsView}
              selected={selected(pane)}
              outputPane={pane.outputPane}
              theme
              dispatch
              uiFont
              editorFont
              diagnosticDispatch={msg => dispatch(DiagnosticsList(msg))}
              locationsDispatch={msg => dispatch(LocationsList(msg))}
              notificationsDispatch={msg => dispatch(NotificationsList(msg))}
              outputDispatch={msg => dispatch(OutputPane(msg))}
              workingDirectory
            />
          </View>
        </View>;
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

  let closePane =
    define(
      // TODO: Is there a VSCode equivalent?
      "workbench.actions.pane.close",
      Command(ClosePane),
    );
};

module Keybindings = {
  open Feature_Input.Schema;
  let toggleProblems =
    bind(
      ~key="<S-C-M>",
      ~command=Commands.problems.id,
      ~condition=WhenExpr.Value(True),
    );

  let toggleProblemsOSX =
    bind(
      ~key="<D-S-M>",
      ~command=Commands.problems.id,
      ~condition="isMac" |> WhenExpr.parse,
    );

  let escKey =
    bind(
      ~key="<ESC>",
      ~command=Commands.closePane.id,
      ~condition="paneFocus" |> WhenExpr.parse,
    );
};

module Contributions = {
  let commands = (~isFocused, model) => {
    let common = Commands.[problems, closePane];
    let vimWindowCommands =
      Component_VimWindows.Contributions.commands
      |> List.map(Oni_Core.Command.map(msg => VimWindowNav(msg)));

    let diagnosticsCommands =
      (
        isFocused && model.selected == Diagnostics
          ? Component_VimTree.Contributions.commands : []
      )
      |> List.map(Oni_Core.Command.map(msg => DiagnosticsList(msg)));

    let locationsCommands =
      (
        isFocused && model.selected == Locations
          ? Component_VimTree.Contributions.commands : []
      )
      |> List.map(Oni_Core.Command.map(msg => LocationsList(msg)));

    let outputCommands =
      (
        isFocused && model.selected == Output
          ? Component_Output.Contributions.commands : []
      )
      |> List.map(Oni_Core.Command.map(msg => OutputPane(msg)));

    let notificationsCommands =
      (
        isFocused && model.selected == Notifications
          ? Component_VimList.Contributions.commands : []
      )
      |> List.map(Oni_Core.Command.map(msg => NotificationsList(msg)));

    isFocused
      ? common
        @ vimWindowCommands
        @ diagnosticsCommands
        @ locationsCommands
        @ notificationsCommands
        @ outputCommands
      : common;
  };

  let contextKeys = (~isFocused, model) => {
    open WhenExpr.ContextKeys;
    let vimNavKeys =
      isFocused
        ? Component_VimWindows.Contributions.contextKeys(
            model.vimWindowNavigation,
          )
        : empty;

    let diagnosticsKeys =
      isFocused && model.selected == Diagnostics
        ? Component_VimTree.Contributions.contextKeys(model.diagnosticsView)
        : empty;

    let locationsKeys =
      isFocused && model.selected == Locations
        ? Component_VimTree.Contributions.contextKeys(model.locationsView)
        : empty;

    let outputKeys =
      isFocused && model.selected == Output
        ? model.outputPane
          |> Option.map(outputPane =>
               Component_Output.Contributions.contextKeys(outputPane)
             )
          |> Option.value(~default=empty)
        : empty;

    let notificationsKeys =
      isFocused && model.selected == Notifications
        ? Component_VimList.Contributions.contextKeys(
            model.notificationsView,
          )
        : empty;

    let activePanel =
      (
        isFocused
          ? [
            Schema.string("activePanel", model =>
              switch (model.selected) {
              | Diagnostics => "workbench.panel.markers"
              | Output => "workbench.panel.output"
              | Notifications => "workbench.panel.notifications"
              | Locations => "workbench.panel.locations"
              }
            ),
          ]
          : []
      )
      |> Schema.fromList
      |> fromSchema(model);

    let paneFocus =
      [Schema.bool("paneFocus", (_: model) => isFocused)]
      |> Schema.fromList
      |> fromSchema(model);

    [
      activePanel,
      paneFocus,
      vimNavKeys,
      diagnosticsKeys,
      locationsKeys,
      notificationsKeys,
      outputKeys,
    ]
    |> unionMany;
  };

  let keybindings = Keybindings.[toggleProblems, toggleProblemsOSX, escKey];
};
