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

let truncateFilepath = path =>
  switch (path) {
  | Some(p) => Filename.basename(p)
  | None => "untitled"
  };

let getBufferMetadata = (buffer: option(Buffer.t)) => {
  switch (buffer) {
  | None => (false, "untitled")
  | Some(v) =>
    let filePath = Buffer.getFilePath(v);
    let modified = Buffer.isModified(v);

    let title = filePath |> truncateFilepath;
    (modified, title);
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
      let (modified, title) =
        Model.Buffers.getBuffer(v.bufferId, buffers) |> getBufferMetadata;

      let renderer = Model.BufferRenderers.getById(v.bufferId, renderers);

      let ret: Tabs.tabInfo = {
        editorId: v.editorId,
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
  let theme = state.theme;
  let mode = state.mode;

  let style = editorViewStyle(theme.background, theme.foreground);

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
    // TODO: Handle show tabs
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
        | BufferRenderer.Editor =>
          let buffer =
            Selectors.getBufferForEditor(state, editor)
            |> Option.value(~default=Buffer.empty);
          let rulers =
            Configuration.getValue(c => c.editorRulers, state.configuration);
          let showLineNumbers =
            Configuration.getValue(
              c => c.editorLineNumbers,
              state.configuration,
            );
          let showMinimap =
            Configuration.getValue(
              c => c.editorMinimapEnabled,
              state.configuration,
            );
          let maxMinimapCharacters =
            Configuration.getValue(
              c => c.editorMinimapMaxColumn,
              state.configuration,
            );
          let matchingPairsEnabled =
            Selectors.getConfigurationValue(state, buffer, c =>
              c.editorMatchBrackets
            );
          let shouldRenderWhitespace =
            Configuration.getValue(
              c => c.editorRenderWhitespace,
              state.configuration,
            );
          let shouldRenderIndentGuides =
            Configuration.getValue(
              c => c.editorRenderIndentGuides,
              state.configuration,
            );
          let shouldHighlightActiveIndentGuides =
            Configuration.getValue(
              c => c.editorHighlightActiveIndentGuide,
              state.configuration,
            );
          let showMinimapSlider =
            Configuration.getValue(
              c => c.editorMinimapShowSlider,
              state.configuration,
            );
          let hoverDelay =
            Configuration.getValue(
              c => c.editorHoverDelay,
              state.configuration,
            )
            |> Revery.Time.ms;
          let isHoverEnabled =
            Configuration.getValue(
              c => c.editorHoverEnabled,
              state.configuration,
            );

          <EditorSurface
            isActiveSplit=isActive
            metrics
            editor
            buffer
            onCursorChange
            onDimensionsChanged={_ => ()}
            onScroll
            theme
            rulers
            showLineNumbers
            editorFont={state.editorFont}
            mode
            showMinimap
            maxMinimapCharacters
            matchingPairsEnabled
            bufferHighlights={state.bufferHighlights}
            bufferSyntaxHighlights={state.bufferSyntaxHighlights}
            diagnostics={state.diagnostics}
            completions={state.completions}
            tokenTheme={state.tokenTheme}
            definition={state.definition}
            shouldRenderWhitespace
            showMinimapSlider
            hoverDelay
            isHoverEnabled
            shouldRenderIndentGuides
            shouldHighlightActiveIndentGuides
          />;
        | BufferRenderer.Welcome => <WelcomeView state />
        | BufferRenderer.Terminal({id}) =>
          state.terminals
          |> Feature_Terminal.getTerminalOpt(id)
          |> Option.map(terminal => {
               <TerminalView editorFont={state.editorFont} metrics terminal />
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
