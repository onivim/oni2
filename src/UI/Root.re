/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery.UI;
open Oni_Model;

module ResizeHandle = Oni_Components.ResizeHandle;
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
    if (Revery.Environment.isWindows && windowDisplayMode == State.Maximized) {
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
  let State.{uiFont as font, sideBar, zenMode, buffers, editorFont, _} = state;

  let theme = Feature_Theme.colors(state.colorTheme);

  let mode = ModeManager.current(state);

  let config = Selectors.configResolver(state);

  let maybeActiveBuffer = Oni_Model.Selectors.getActiveBuffer(state);
  let activeEditor = Feature_Layout.activeEditor(state.layout);
  let indentationSettings =
    maybeActiveBuffer
    |> Option.map(Oni_Core.Buffer.getIndentation)
    |> Option.value(~default=Oni_Core.IndentationSettings.default);

  let statusBarDispatch = msg => dispatch(Actions.StatusBar(msg));
  let messagesDispatch = msg => dispatch(Actions.Messages(msg));

  let messages = () => {
    <Feature_Messages.View
      theme
      font={state.uiFont}
      model={state.messages}
      dispatch=messagesDispatch
    />;
  };

  let statusBar = () =>
    if (Feature_StatusBar.Configuration.visible.get(config) && !zenMode) {
      <View style=Styles.statusBar>
        <Feature_StatusBar.View
          mode
          subMode={Feature_Vim.subMode(state.vim)}
          recordingMacro={state.vim |> Feature_Vim.recordingMacro}
          notifications={state.notifications}
          diagnostics={state.diagnostics}
          font={state.uiFont}
          scm={state.scm}
          statusBar={state.statusBar}
          activeBuffer=maybeActiveBuffer
          activeEditor={Some(activeEditor)}
          indentationSettings
          theme
          dispatch=statusBarDispatch
          workingDirectory={Feature_Workspace.workingDirectory(
            state.workspace,
          )}
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
      <Dock font={state.uiFont} theme sideBar extensions={state.extensions} />;
    } else {
      React.empty;
    };

  let sideBar = () =>
    if (!zenMode) {
      <SideBarView config theme state dispatch />;
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
        workingDirectory={Feature_Workspace.workingDirectory(state.workspace)}
        theme
        font
        dispatch
      />;

    | None => React.empty
    };
  };

  let titleDispatch = msg => dispatch(Actions.TitleBar(msg));
  let registrationDispatch = msg => dispatch(Actions.Registration(msg));

  let mapDisplayMode =
    fun
    | Oni_Model.State.Minimized => Feature_TitleBar.Minimized
    | Oni_Model.State.Maximized => Feature_TitleBar.Maximized
    | Oni_Model.State.Windowed => Feature_TitleBar.Windowed
    | Oni_Model.State.Fullscreen => Feature_TitleBar.Fullscreen;

  let defaultSurfaceComponents = [
    <activityBar />,
    <sideBar />,
    <EditorView state theme dispatch />,
  ];

  let surfaceComponents =
    switch (Feature_SideBar.location(state.sideBar)) {
    | Feature_SideBar.Left => defaultSurfaceComponents
    | Feature_SideBar.Right => List.rev(defaultSurfaceComponents)
    };

  let context = Oni_Model.ContextKeys.all(state);

  let menuBarElement =
    switch (Feature_MenuBar.Configuration.visibility.get(config)) {
    | `visible =>
      <Feature_MenuBar.View
        isWindowFocused={state.windowIsFocused}
        font={state.uiFont}
        config
        context
        input={state.input}
        theme
        model={state.menuBar}
        dispatch={msg => dispatch(Actions.MenuBar(msg))}
      />
    | `hidden => React.empty
    };

  let zoom = Feature_Zoom.zoom(state.zoom);
  // Correct for zoom in title bar height
  let titlebarHeight = state.titlebarHeight /. zoom;

  <View style={Styles.root(theme, state.windowDisplayMode)}>
    <Feature_TitleBar.View
      menuBar=menuBarElement
      activeBuffer=maybeActiveBuffer
      workspaceRoot={Feature_Workspace.rootName(state.workspace)}
      workspaceDirectory={Feature_Workspace.workingDirectory(state.workspace)}
      registration={state.registration}
      config
      isFocused={state.windowIsFocused}
      windowDisplayMode={state.windowDisplayMode |> mapDisplayMode}
      font={state.uiFont}
      theme
      dispatch=titleDispatch
      registrationDispatch
      height=titlebarHeight
    />
    <View style=Styles.workspace>
      <View style=Styles.surface>
        {React.listToElement(surfaceComponents)}
      </View>
      <Feature_Pane.View
        config
        isFocused={FocusManager.current(state) == Focus.Pane}
        iconTheme={state.iconTheme}
        languageInfo={state.languageInfo}
        theme
        editorFont
        uiFont
        dispatch={msg => dispatch(Actions.Pane(msg))}
        pane={state.pane}
        workingDirectory={Feature_Workspace.workingDirectory(state.workspace)}
      />
    </View>
    <Overlay>
      {switch (state.quickmenu) {
       | None => React.empty
       | Some(quickmenu) =>
         <QuickmenuView theme config state=quickmenu font />
       }}
      <Feature_Input.View.Overlay
        input={state.input}
        uiFont
        bottom=50
        right=50
      />
      <Feature_Registers.View
        theme
        registers={state.registers}
        font
        dispatch={msg => dispatch(Actions.Registers(msg))}
      />
      <Feature_Registration.View.Modal
        theme
        registration={state.registration}
        font
        dispatch={msg => dispatch(Actions.Registration(msg))}
      />
    </Overlay>
    <statusBar />
    <Component_ContextMenu.View.Overlay />
    <Tooltip.Overlay theme font=uiFont />
    <messages />
    <modals />
    <Overlay>
      <Feature_Sneak.View.Overlay model={state.sneak} theme font />
    </Overlay>
    {Revery.Environment.isWindows && state.windowDisplayMode != State.Maximized
       ? <WindowResizers /> : React.empty}
  </View>;
};
