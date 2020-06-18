/*
 * Editor.re
 *
 * Editor component - an 'Editor' encapsulates the following:
 * - a 'tabbar'
 * - an editor surface - usually a textual buffer view
 */

open Revery.UI;
open Rench;
open Oni_Core;
open Oni_Model;
open Actions;

module Colors = Feature_Theme.Colors;

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

      <Feature_Editor.EditorSurface
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
      let renderHover = (~gutterWidth) =>
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
    flexGrow(1),
    flexDirection(`Column),
  ];
};

let make =
    (~state: State.t, ~dispatch: Oni_Model.Actions.t => unit, ~theme, ()) => {
  let onFileDropped = ({paths, _}: NodeEvents.fileDropEventParams) =>
    dispatch(Actions.FilesDropped({paths: paths}));

  module ContentProvider = {
    open Feature_Editor;

    type t = Editor.t;

    let id = editor => Editor.getId(editor);

    open {
           let getBufferMetadata = (buffer: option(Buffer.t)) => {
             switch (buffer) {
             | None => (false, "untitled", "untitled")
             | Some(buffer) =>
               let filePath =
                 Buffer.getFilePath(buffer)
                 |> Option.value(~default="untitled");
               let modified = Buffer.isModified(buffer);

               let title = filePath |> Filename.basename;
               (modified, title, filePath);
             };
           };
         };

    let title = editor => {
      let (_, title, _) =
        Buffers.getBuffer(Editor.getBufferId(editor), state.buffers)
        |> getBufferMetadata;

      let renderer =
        BufferRenderers.getById(
          Editor.getBufferId(editor),
          state.bufferRenderers,
        );

      switch (renderer) {
      | Welcome => "Welcome"
      | Terminal({title, _}) => title
      | _ => Path.filename(title)
      };
    };

    let isActive = _ => true; // TODO
    let isModified = editor => {
      let (modified, _, _) =
        Buffers.getBuffer(Editor.getBufferId(editor), state.buffers)
        |> getBufferMetadata;

      modified;
    };

    let icon = editor => {
      let (_, _, filePath) =
        Buffers.getBuffer(Editor.getBufferId(editor), state.buffers)
        |> getBufferMetadata;

      let language =
        Ext.LanguageInfo.getLanguageFromFilePath(
          state.languageInfo,
          filePath,
        );

      IconTheme.getIconForFile(state.iconTheme, filePath, language);
    };

    let render = editor =>
      <Parts.EditorContainer editor state theme isActive=true dispatch />;
  };

  <View onFileDropped style={Styles.container(theme)}>
    <View style={Styles.container(theme)}>
      <Feature_Layout.View
        uiFont={state.uiFont}
        theme
        model={state.layout}
        dispatch={msg => dispatch(Actions.Layout(msg))}>
        ...(module ContentProvider)
      </Feature_Layout.View>
    </View>
  </View>;
};
