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
module OptionEx = Oni_Core.Utility.OptionEx;
open Actions;
open Feature_Editor;

module Colors = Feature_Theme.Colors;

module Parts = {
  module Editor = {
    let make =
        (
          ~editor,
          ~buffer,
          ~state: State.t,
          ~theme,
          ~isActive,
          ~backgroundColor=?,
          ~foregroundColor=?,
          ~showDiffMarkers=true,
          ~dispatch,
          ~renderOverlays,
          (),
        ) => {
      let languageConfiguration =
        buffer
        |> Oni_Core.Buffer.getFileType
        |> Oni_Core.Buffer.FileType.toString
        |> Exthost.LanguageInfo.getLanguageConfiguration(state.languageInfo)
        |> Option.value(~default=LanguageConfiguration.default);

      let editorDispatch = msg =>
        dispatch(
          Editor({scope: EditorScope.Editor(Editor.getId(editor)), msg}),
        );
      let onEditorSizeChanged = (editorId, pixelWidth, pixelHeight) =>
        dispatch(EditorSizeChanged({id: editorId, pixelWidth, pixelHeight}));

      <EditorSurface
        key={editor |> Feature_Editor.Editor.key}
        dispatch=editorDispatch
        ?backgroundColor
        ?foregroundColor
        showDiffMarkers
        isActiveSplit=isActive
        editor
        buffer
        uiFont={state.uiFont}
        languageConfiguration
        languageInfo={state.languageInfo}
        grammarRepository={state.grammarRepository}
        onEditorSizeChanged
        theme
        vim={state.vim}
        bufferSyntaxHighlights={state.syntaxHighlights}
        diagnostics={state.diagnostics}
        buffers={state.buffers}
        snippets={state.snippets}
        tokenTheme={state.colorTheme |> Feature_Theme.tokenColors}
        languageSupport={state.languageSupport}
        windowIsFocused={state.windowIsFocused}
        perFileTypeConfig={Feature_Configuration.resolver(
          state.config,
          state.vim,
        )}
        renderOverlays
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
        Selectors.getBufferForEditor(state.buffers, editor)
        |> OptionEx.value_or_lazy(() => Buffer.empty(~font=state.editorFont));

      let renderOverlays = (~gutterWidth as _) => React.empty;

      let isDark =
        state.colorTheme |> Feature_Theme.variant != ColorTheme.Light;

      switch (renderer) {
      | Terminal({insertMode, _}) when !insertMode =>
        let backgroundColor = Feature_Terminal.defaultBackground(theme);
        let foregroundColor = Feature_Terminal.defaultForeground(theme);

        <Editor
          editor
          buffer
          state
          theme
          isActive
          backgroundColor
          foregroundColor
          showDiffMarkers=false
          dispatch
          renderOverlays
        />;

      | Image =>
        buffer
        |> Oni_Core.Buffer.getFilePath
        |> Option.map(filePath => {<Feature_ImagePreview.View filePath />})
        |> Option.value(~default=<Text text="Unable to load." />)

      | Terminal({id, _}) =>
        state.terminals
        |> Feature_Terminal.getTerminalOpt(id)
        |> Option.map(terminal => {
             let config = Selectors.configResolver(state);
             <TerminalView
               config
               isActive
               theme
               font={state.terminalFont}
               terminal
             />;
           })
        |> Option.value(~default=React.empty)

      | Editor =>
        <Editor editor buffer state theme isActive dispatch renderOverlays />

      | Welcome => <WelcomeView isDark theme uiFont editorFont />

      | Version => <VersionView theme uiFont editorFont />

      | FullChangelog =>
        <Feature_Changelog.View.Full
          state={state.changelog}
          theme
          dispatch=changelogDispatch
          uiFont
        />

      | ExtensionDetails =>
        <Feature_Extensions.DetailsView
          model={state.extensions}
          tokenTheme={state.colorTheme |> Feature_Theme.tokenColors}
          theme
          font=uiFont
          dispatch={msg => dispatch(Actions.Extensions(msg))}
        />

      | DebugInput => <DebugInputView state />

      | UpdateChangelog => <Feature_Changelog.View.Update theme uiFont />
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

    let preview = editor => Editor.getPreview(editor);

    let title = editor => {
      let (_, title, _) =
        Feature_Buffers.get(Editor.getBufferId(editor), state.buffers)
        |> getBufferMetadata;

      let renderer =
        BufferRenderers.getById(
          Editor.getBufferId(editor),
          state.bufferRenderers,
        );

      switch (renderer) {
      | Welcome => "Welcome"
      | Terminal({title, _}) => title
      | _ => Utility.Path.filename(title)
      };
    };

    let tooltip = editor => {
      let (_, _, filePath) =
        Feature_Buffers.get(Editor.getBufferId(editor), state.buffers)
        |> getBufferMetadata;

      let renderer =
        BufferRenderers.getById(
          Editor.getBufferId(editor),
          state.bufferRenderers,
        );

      switch (renderer) {
      | Welcome => "Welcome"
      | Terminal({title, _}) => title
      | _ => filePath
      };
    };

    let isModified = editor => {
      let (modified, _, _) =
        Feature_Buffers.get(Editor.getBufferId(editor), state.buffers)
        |> getBufferMetadata;

      modified;
    };

    let icon = editor => {
      let buffer =
        Feature_Buffers.get(Editor.getBufferId(editor), state.buffers);
      let (_, _, filePath) = getBufferMetadata(buffer);

      let language =
        switch (buffer) {
        | Some(buf) =>
          Oni_Core.Buffer.getFileType(buf)
          |> Oni_Core.Buffer.FileType.toString
        | None => Oni_Core.Buffer.FileType.default
        };

      IconTheme.getIconForFile(state.iconTheme, filePath, language);
    };

    let render = (~isActive, editor) =>
      <Parts.EditorContainer editor state theme isActive dispatch />;
  };

  let config = Selectors.configResolver(state);

  let editorShowTabs =
    Feature_Configuration.GlobalConfiguration.Workbench.editorShowTabs.get(
      config,
    );

  let hideZenModeTabs = !Feature_Zen.shouldShowTabsInZenMode(state.zen);
  let isZenMode = Feature_Zen.isZen(state.zen);

  let showTabs = editorShowTabs && (!isZenMode || !hideZenModeTabs);

  let isFocused = FocusManager.current(state) |> Focus.isLayoutFocused;

  <View onFileDropped style={Styles.container(theme)}>
    <Feature_Layout.View
      uiFont={state.uiFont}
      theme
      isFocused
      isZenMode
      showTabs
      model={state.layout}
      config
      dispatch={msg => dispatch(Actions.Layout(msg))}>
      ...(module ContentProvider)
    </Feature_Layout.View>
  </View>;
};
