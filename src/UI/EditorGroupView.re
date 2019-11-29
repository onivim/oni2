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

module List = Utility.List;

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

let toUiTabs = (editorGroup: Model.EditorGroup.t, buffers: Model.Buffers.t) => {
  let f = (id: int) => {
    switch (Model.EditorGroup.getEditorById(id, editorGroup)) {
    | None => None
    | Some(v) =>
      let (modified, title) =
        Model.Buffers.getBuffer(v.bufferId, buffers) |> getBufferMetadata;
      let ret: Tabs.tabInfo = {editorId: v.editorId, title, modified};
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

  let children = {
    let maybeEditor = EditorGroup.getActiveEditor(editorGroup);
    let tabs = toUiTabs(editorGroup, state.buffers);
    let uiFont = state.uiFont;

    let metrics = editorGroup.metrics;

    let editorView =
      switch (maybeEditor) {
      | Some(editor) =>
        <EditorSurface
          isActiveSplit=isActive
          editorGroup
          metrics
          editor
          state
        />
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

  <View onMouseDown style>
    <View style=absoluteStyle> children </View>
    <View style=overlayStyle />
  </View>;
};
