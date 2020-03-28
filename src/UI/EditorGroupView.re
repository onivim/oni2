/*
 * Editor.re
 *
 * Editor component - an 'Editor' encapsulates the following:
 * - a 'tabbar'
 * - an editor surface - usually a textual buffer view
 */

open Revery.UI;
open Oni_Core;
open Oni_Model;
module Model = Oni_Model;

module Window = WindowManager;

module Colors = Feature_Theme.Colors;
module EditorSurface = Feature_Editor.EditorSurface;

let noop = () => ();

let editorViewStyle = (background, foreground) =>
  Style.[
    backgroundColor(background),
    color(foreground),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
    flexDirection(`Column),
  ];

let getBufferMetadata = (buffer: option(Buffer.t)) => {
  switch (buffer) {
  | None => (false, "untitled", "untitled")
  | Some(v) =>
    let filePath =
      Buffer.getFilePath(v) |> Option.value(~default="untitled");
    let modified = Buffer.isModified(v);

    let title = filePath |> Filename.basename;
    (modified, title, filePath);
  };
};

let toUiTabs =
    (
      editorGroup: Model.EditorGroup.t,
      buffers: Model.Buffers.t,
      renderers: Model.BufferRenderers.t,
    ) => {
  let f = (id: int) => {
    switch (Model.EditorGroup.getEditorById(id, editorGroup)) {
    | None => None
    | Some(v) =>
      let (modified, title, filePath) =
        Model.Buffers.getBuffer(v.bufferId, buffers) |> getBufferMetadata;

      let renderer = Model.BufferRenderers.getById(v.bufferId, renderers);

      let ret: Tabs.tabInfo = {
        editorId: v.editorId,
        filePath,
        title,
        modified,
        renderer,
      };
      Some(ret);
    };
  };

  List.filter_map(f, editorGroup.reverseTabOrder) |> List.rev;
};

let make = (~state: State.t, ~windowId: int, ~editorGroup: EditorGroup.t, ()) => {
  let theme = Feature_Theme.resolver(state.colorTheme);
  let mode = state.vimMode;

  let style =
    editorViewStyle(
      Colors.Editor.background.from(theme),
      Colors.foreground.from(theme),
    );

  let isActive = EditorGroups.isActive(state.editorGroups, editorGroup);

  let overlayStyle =
    Style.[
      position(`Absolute),
      top(0),
      left(0),
      right(0),
      bottom(0),
      pointerEvents(`Ignore),
      backgroundColor(Revery.Color.rgba(0., 0., 0., isActive ? 0. : 0.1)),
    ];

  let absoluteStyle =
    Style.[position(`Absolute), top(0), left(0), right(0), bottom(0)];

  let showTabs =
    if (state.zenMode) {
      Configuration.getValue(c => !c.zenModeHideTabs, state.configuration);
    } else {
      Configuration.getValue(
        c => c.workbenchEditorShowTabs,
        state.configuration,
      );
    };

  let onDimensionsChanged =
      ({width, height}: NodeEvents.DimensionsChangedEventParams.t) => {
    let height = showTabs ? height - Constants.tabHeight : height;

    GlobalContext.current().notifyEditorSizeChanged(
      ~editorGroupId=editorGroup.editorGroupId,
      ~width,
      ~height,
      (),
    );
  };

  let children = {
    let maybeEditor = EditorGroup.getActiveEditor(editorGroup);
    let tabs = toUiTabs(editorGroup, state.buffers, state.bufferRenderers);
    let uiFont = state.uiFont;

    let metrics = editorGroup.metrics;
    let editorView =
      switch (maybeEditor) {
      | Some(editor) =>
        let onScroll = deltaY => {
          let () =
            GlobalContext.current().editorScrollDelta(
              ~editorId=editor.editorId,
              ~deltaY,
              (),
            );
          ();
        };
        let onCursorChange = cursor =>
          GlobalContext.current().dispatch(
            Actions.EditorCursorMove(editor.editorId, [cursor]),
          );
        let renderer =
          BufferRenderers.getById(editor.bufferId, state.bufferRenderers);
        switch (renderer) {
        | BufferRenderer.Terminal({insertMode, _}) when !insertMode =>
          let buffer =
            Selectors.getBufferForEditor(state, editor)
            |> Option.value(~default=Buffer.initial);

          <EditorSurface
            isActiveSplit=isActive
            metrics
            editor
            buffer
            onCursorChange
            onDimensionsChanged={_ => ()}
            onScroll
            theme
            mode
            bufferHighlights={state.bufferHighlights}
            bufferSyntaxHighlights={state.syntaxHighlights}
            diagnostics={state.diagnostics}
            completions={state.completions}
            tokenTheme={state.tokenTheme}
            definition={state.definition}
            windowIsFocused={state.windowIsFocused}
            config={Feature_Configuration.resolver(state.config)}
          />;
        | BufferRenderer.Editor =>
          let buffer =
            Selectors.getBufferForEditor(state, editor)
            |> Option.value(~default=Buffer.initial);

          <EditorSurface
            isActiveSplit=isActive
            metrics
            editor
            buffer
            onCursorChange
            onDimensionsChanged={_ => ()}
            onScroll
            theme
            mode
            bufferHighlights={state.bufferHighlights}
            bufferSyntaxHighlights={state.syntaxHighlights}
            diagnostics={state.diagnostics}
            completions={state.completions}
            tokenTheme={state.tokenTheme}
            definition={state.definition}
            windowIsFocused={state.windowIsFocused}
            config={Feature_Configuration.resolver(state.config)}
          />;
        | BufferRenderer.Welcome => <WelcomeView state />
        | BufferRenderer.Terminal({id, _}) =>
          state.terminals
          |> Feature_Terminal.getTerminalOpt(id)
          |> Option.map(terminal => {
               <TerminalView
                 theme
                 font={state.terminalFont}
                 metrics
                 terminal
               />
             })
          |> Option.value(~default=React.empty)
        };
      | None => React.empty
      };

    switch (showTabs) {
    | false => editorView
    | true =>
      React.listToElement([
        <Tabs
          active=isActive
          activeEditorId={editorGroup.activeEditorId}
          theme
          tabs
          mode
          uiFont
          languageInfo={state.languageInfo}
          iconTheme={state.iconTheme}
        />,
        editorView,
      ])
    };
  };

  let onMouseDown = _ => {
    GlobalContext.current().setActiveWindow(
      windowId,
      editorGroup.editorGroupId,
    );
  };

  <View onMouseDown style onDimensionsChanged>
    <View style=absoluteStyle> children </View>
    <View style=overlayStyle />
  </View>;
};
