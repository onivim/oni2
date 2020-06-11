open Revery.UI;
open Oni_Core;
open Oni_Model;
open Actions;
module Model = Oni_Model;

module Colors = Feature_Theme.Colors;
module EditorSurface = Feature_Editor.EditorSurface;

let getBufferMetadata = (buffer: option(Buffer.t)) => {
  switch (buffer) {
  | None => (false, "untitled", "untitled")
  | Some(buffer) =>
    let filePath =
      Buffer.getFilePath(buffer) |> Option.value(~default="untitled");
    let modified = Buffer.isModified(buffer);

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
    | Some(editor) =>
      open Feature_Editor;
      let (modified, title, filePath) =
        Model.Buffers.getBuffer(Editor.getBufferId(editor), buffers)
        |> getBufferMetadata;

      let renderer =
        Model.BufferRenderers.getById(Editor.getBufferId(editor), renderers);

      Some(
        Tabs.{editorId: editor.editorId, filePath, title, modified, renderer},
      );
    };
  };

  List.filter_map(f, editorGroup.reverseTabOrder) |> List.rev;
};

module Parts = {
  module Editor = {
    let make =
        (
          ~editor,
          ~state: State.t,
          ~theme,
          ~isActive,
          ~backgroundColor=?,
          ~foregroundColor=?,
          ~showDiffMarkers=true,
          ~renderHover,
          ~dispatch,
          (),
        ) => {
      let buffer =
        Selectors.getBufferForEditor(state, editor)
        |> Option.value(~default=Buffer.initial);

      let onEditorSizeChanged = (editorId, pixelWidth, pixelHeight) =>
        dispatch(EditorSizeChanged({id: editorId, pixelWidth, pixelHeight}));
      let onCursorChange = cursor =>
        dispatch(EditorCursorMove(editor.editorId, [cursor]));

      let editorDispatch = editorMsg =>
        dispatch(Editor({editorId: editor.editorId, msg: editorMsg}));

      <EditorSurface
        dispatch=editorDispatch
        ?backgroundColor
        ?foregroundColor
        showDiffMarkers
        isActiveSplit=isActive
        editor
        buffer
        onCursorChange
        onEditorSizeChanged
        theme
        mode={Feature_Vim.mode(state.vim)}
        bufferHighlights={state.bufferHighlights}
        bufferSyntaxHighlights={state.syntaxHighlights}
        diagnostics={state.diagnostics}
        completions={state.completions}
        tokenTheme={state.tokenTheme}
        definition={state.definition}
        windowIsFocused={state.windowIsFocused}
        config={Feature_Configuration.resolver(state.config)}
        renderHover
      />;
    };
  };

  module EditorContainer = {
    let make =
        (
          ~editor: Feature_Editor.Editor.t,
          ~state: State.t,
          ~theme,
          ~isActive,
          ~dispatch,
          (),
        ) => {
      let State.{uiFont, editorFont, _} = state;

      let renderer =
        BufferRenderers.getById(
          Feature_Editor.Editor.getBufferId(editor),
          state.bufferRenderers,
        );

      let changelogDispatch = msg => dispatch(Changelog(msg));

      let buffer =
        Selectors.getBufferForEditor(state, editor)
        |> Option.value(~default=Buffer.initial);
      let renderHover = (~gutterWidth, ~cursorOffset) =>
        <Feature_Hover.View
          colorTheme=theme
          tokenTheme={state.tokenTheme}
          model={state.hover}
          uiFont={state.uiFont}
          editorFont={state.editorFont}
          languageInfo={state.languageInfo}
          grammars={state.grammarRepository}
          diagnostics={state.diagnostics}
          editor
          buffer
          gutterWidth
          cursorOffset
        />;

      switch (renderer) {
      | Terminal({insertMode, _}) when !insertMode =>
        let backgroundColor = Feature_Terminal.defaultBackground(theme);
        let foregroundColor = Feature_Terminal.defaultForeground(theme);

        <Editor
          editor
          state
          theme
          isActive
          backgroundColor
          foregroundColor
          showDiffMarkers=false
          dispatch
          renderHover
        />;

      | Terminal({id, _}) =>
        state.terminals
        |> Feature_Terminal.getTerminalOpt(id)
        |> Option.map(terminal => {
             <TerminalView theme font={state.terminalFont} terminal />
           })
        |> Option.value(~default=React.empty)

      | Editor => <Editor editor state theme isActive dispatch renderHover />

      | Welcome => <WelcomeView theme uiFont editorFont />

      | Version => <VersionView theme uiFont editorFont />

      | FullChangelog =>
        <Feature_Changelog.View.Full
          state={state.changelog}
          theme
          dispatch=changelogDispatch
          uiFont
        />

      | UpdateChangelog({since}) =>
        <Feature_Changelog.View.Update since theme uiFont />
      };
    };
  };
};

module Styles = {
  open Style;

  let container = theme => [
    backgroundColor(Colors.Editor.background.from(theme)),
    color(Colors.foreground.from(theme)),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
  ];

  let editorContainer = [flexGrow(1), flexDirection(`Column)];
};

let make =
    (~state: State.t, ~theme, ~editorGroup: EditorGroup.t, ~dispatch, ()) => {
  let State.{vim, uiFont, editorFont, _} = state;

  let mode = Feature_Vim.mode(vim);

  let isActive = EditorGroups.isActive(state.editorGroups, editorGroup);

  let showTabs =
    if (state.zenMode) {
      Configuration.getValue(c => !c.zenModeHideTabs, state.configuration);
    } else {
      Configuration.getValue(
        c => c.workbenchEditorShowTabs,
        state.configuration,
      );
    };

  let children = {
    let editorContainer =
      switch (EditorGroup.getActiveEditor(editorGroup)) {
      | Some(editor) =>
        <Parts.EditorContainer editor state theme isActive dispatch />
      | None => <WelcomeView theme editorFont uiFont />
      };

    if (showTabs) {
      let tabs =
        <Tabs
          active=isActive
          activeEditorId={editorGroup.activeEditorId}
          theme
          tabs={toUiTabs(editorGroup, state.buffers, state.bufferRenderers)}
          mode
          uiFont
          languageInfo={state.languageInfo}
          iconTheme={state.iconTheme}
          dispatch
        />;

      <View style=Styles.editorContainer> tabs editorContainer </View>;
    } else {
      editorContainer;
    };
  };

  let onMouseDown = _ =>
    dispatch(EditorGroupSelected(editorGroup.editorGroupId));

  <View onMouseDown style={Styles.container(theme)}> children </View>;
};
