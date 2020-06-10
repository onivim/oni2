/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery.UI;
open Oni_Model;

module ContextMenu = Oni_Components.ContextMenu;
module KeyDisplayer = Oni_Components.KeyDisplayer;
module Tooltip = Oni_Components.Tooltip;

module Colors = Feature_Theme.Colors;

module Constants = {
  let statusBarHeight = 25;
  let titleBarHeight = 22;
};

module Styles = {
  open Style;

  let root = (theme, windowDisplayMode) => {
    let style =
      ref([
        backgroundColor(Colors.Editor.background.from(theme)),
        color(Colors.foreground.from(theme)),
        position(`Absolute),
        top(0),
        left(0),
        right(0),
        bottom(0),
        justifyContent(`Center),
        alignItems(`Stretch),
      ]);
    if (Revery.Environment.os == Windows
        && windowDisplayMode == State.Maximized) {
      style := [margin(6), ...style^];
    };
    style^;
  };

  let surface = [flexGrow(1), flexDirection(`Row)];

  let workspace = Style.[flexGrow(1), flexDirection(`Column)];

  let statusBar = [
    Style.height(Constants.statusBarHeight),
    justifyContent(`Center),
    alignItems(`Center),
  ];

  let titleBar = background =>
    Style.[
      flexGrow(0),
      height(Constants.titleBarHeight),
      backgroundColor(background),
    ];
};

let make = (~dispatch, ~state: State.t, ()) => {
  let State.{
        configuration,
        contextMenu,
        uiFont as font,
        editorFont,
        sideBar,
        zenMode,
        pane,
        buffers,
        _,
      } = state;

  let theme = Feature_Theme.colors(state.colorTheme);

  let mode = ModeManager.current(state);

  let maybeActiveBuffer = Oni_Model.Selectors.getActiveBuffer(state);
  let maybeActiveEditor =
    Oni_Model.EditorGroups.getActiveEditor(state.editorGroups);
  let indentationSettings = Oni_Model.Indentation.getForActiveBuffer(state);

  let statusBarDispatch = msg => dispatch(Actions.StatusBar(msg));

  let statusBar = () =>
    if (Selectors.getActiveConfigurationValue(state, c =>
          c.workbenchStatusBarVisible
        )
        && !zenMode) {
      <View style=Styles.statusBar>
        <Feature_StatusBar.View
          mode
          notifications={state.notifications}
          contextMenu
          diagnostics={state.diagnostics}
          font={state.uiFont}
          statusBar={state.statusBar}
          activeBuffer=maybeActiveBuffer
          activeEditor=maybeActiveEditor
          indentationSettings
          theme
          dispatch=statusBarDispatch
        />
      </View>;
    } else {
      React.empty;
    };

  let activityBar = () =>
    if (Selectors.getActiveConfigurationValue(state, c =>
          c.workbenchActivityBarVisible
        )
        && !zenMode) {
      React.listToElement([
        <Dock theme sideBar pane />,
        <WindowHandle direction=`Vertical />,
      ]);
    } else {
      React.empty;
    };

  let sideBar = () =>
    if (!zenMode && sideBar.isOpen) {
      React.listToElement([
        <SideBarView theme state />,
        <WindowHandle direction=`Vertical />,
      ]);
    } else {
      React.empty;
    };

  let modals = () => {
    switch (state.modal) {
    | Some(model) =>
      let dispatch = msg => dispatch(Actions.Modals(msg));

      <Feature_Modals.View
        model
        buffers
        workingDirectory={state.workspace.workingDirectory}
        theme
        font
        dispatch
      />;

    | None => React.empty
    };
  };

  let contextMenuOverlay = () => {
    let onClick = () => dispatch(ContextMenuOverlayClicked);

    <ContextMenu.Overlay onClick />;
  };

  let titleDispatch = msg =>
    GlobalContext.current().dispatch(Actions.TitleBar(msg));

  let mapDisplayMode =
    fun
    | Oni_Model.State.Minimized => Feature_TitleBar.Minimized
    | Oni_Model.State.Maximized => Feature_TitleBar.Maximized
    | Oni_Model.State.Windowed => Feature_TitleBar.Windowed
    | Oni_Model.State.Fullscreen => Feature_TitleBar.Fullscreen;

  <View style={Styles.root(theme, state.windowDisplayMode)}>
    <Feature_TitleBar.View
      isFocused={state.windowIsFocused}
      windowDisplayMode={state.windowDisplayMode |> mapDisplayMode}
      font={state.uiFont}
      title={state.windowTitle}
      theme
      dispatch=titleDispatch
    />
    <View style=Styles.workspace>
      <View style=Styles.surface>
        <activityBar />
        <sideBar />
        <EditorView state theme dispatch />
      </View>
      <PaneView theme uiFont editorFont state />
    </View>
    <Overlay>
      {switch (state.quickmenu) {
       | None => React.empty
       | Some(quickmenu) =>
         <QuickmenuView theme configuration state=quickmenu font />
       }}
      {switch (state.keyDisplayer) {
       | Some(model) => <KeyDisplayer model uiFont bottom=50 right=50 />
       | None => React.empty
       }}
    </Overlay>
    <statusBar />
    <contextMenuOverlay />
    <Tooltip.Overlay theme font=uiFont />
    <modals />
    <Overlay>
      <Feature_Sneak.View.Overlay model={state.sneak} theme font />
    </Overlay>
    {Revery.Environment.os == Windows
     && state.windowDisplayMode != State.Maximized
       ? <WindowResizers /> : React.empty}
  </View>;
};
