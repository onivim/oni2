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

let noop = () => ();

let component = React.component("EditorGroupView");

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
    open Vim.BufferMetadata;
    let {filePath, modified, _} = Buffer.getMetadata(v);

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
      let ret: Tabs.tabInfo = {
        title,
        modified,
        active: EditorGroup.isActiveEditor(editorGroup, v.editorId),
        onClick: () => GlobalContext.current().openEditorById(v.editorId),
        onClose: () => GlobalContext.current().closeEditorById(v.editorId),
      };
      Some(ret);
    };
  };

  Utility.filterMap(f, editorGroup.reverseTabOrder) |> List.rev;
};

let createElement =
    (~state: State.t, ~windowId: int, ~editorGroupId: int, ~children as _, ()) =>
  component(hooks => {
    let theme = state.theme;
    let mode = state.mode;

    let editorGroup = Selectors.getEditorGroupById(state, editorGroupId);
    let style =
      editorViewStyle(theme.colors.background, theme.colors.foreground);

    let isActive =
      switch (editorGroup) {
      | None => false
      | Some(v) => v.editorGroupId == state.editorGroups.activeId
      };

    let overlayStyle =
      Style.[
        position(`Absolute),
        top(0),
        left(0),
        right(0),
        bottom(0),
        pointerEvents(`Ignore),
        backgroundColor(
          Revery.Color.rgba(0.0, 0., 0., isActive ? 0.0 : 0.1),
        ),
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

    let children =
      switch (editorGroup) {
      | None => [React.empty]
      | Some(v) =>
        let editor = Some(v) |> Selectors.getActiveEditor;
        let tabs = toUiTabs(v, state.buffers);
        let uiFont = state.uiFont;

        let metrics = v.metrics;

        let editorView =
          switch (editor) {
          | Some(v) =>
            <EditorSurface
              isActiveSplit=isActive
              editorGroupId
              metrics
              editor=v
              state
            />
          | None => React.empty
          };
        switch (showTabs) {
        | false => [editorView]
        | true => [
            <Tabs active=isActive theme tabs mode uiFont />,
            editorView,
          ]
        };
      };

    let onMouseDown = _ => {
      GlobalContext.current().setActiveWindow(windowId, editorGroupId);
    };

    (
      hooks,
      <View onMouseDown style>
        <View style=absoluteStyle> ...children </View>
        <View style=overlayStyle />
      </View>,
    );
  });
