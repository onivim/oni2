module BracketMatch = BracketMatch;
module BufferLineColorizer = BufferLineColorizer;
module BufferViewTokenizer = BufferViewTokenizer;
module Selection = Selection;

module Editor = Editor;
module EditorBuffer = EditorBuffer;
module EditorId = EditorId;
module EditorLayout = EditorLayout;
module EditorSurface = EditorSurface;

module EditorDiffMarkers = EditorDiffMarkers;

module Wrapping = Wrapping;

module Contributions = {
  let configuration = EditorConfiguration.contributions;
};

open Msg;

[@deriving show({with_path: false})]
type msg = Msg.t;

type outmsg =
  | Nothing
  | MouseHovered(EditorCoreTypes.Location.t)
  | MouseMoved(EditorCoreTypes.Location.t);

type model = Editor.t;

module Constants = {
  let editorWheelMultiplier = 50.;
  let minimapWheelMultiplier = 150.;
  let scrollbarWheelMultiplier = 300.;
};

let update = (editor, msg) => {
  switch (msg) {
  | VerticalScrollbarAfterTrackClicked({newPixelScrollY})
  | VerticalScrollbarBeforeTrackClicked({newPixelScrollY})
  | VerticalScrollbarMouseDrag({newPixelScrollY}) => (
      Editor.scrollToPixelY(~pixelY=newPixelScrollY, editor),
      Nothing,
    )
  | MinimapMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelY(
        ~pixelY=deltaWheel *. Constants.minimapWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | MinimapClicked({viewLine}) => (
      Editor.scrollToLine(~line=viewLine, editor),
      Nothing,
    )
  | MinimapDragged({newPixelScrollY}) => (
      Editor.scrollToPixelY(~pixelY=newPixelScrollY, editor),
      Nothing,
    )
  | EditorMouseWheel({deltaX, deltaY}) => (
      Editor.scrollDeltaPixelXY(
        ~pixelX=deltaX *. Constants.editorWheelMultiplier,
        ~pixelY=deltaY *. Constants.editorWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | VerticalScrollbarMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelY(
        ~pixelY=deltaWheel *. Constants.scrollbarWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | HorizontalScrollbarBeforeTrackClicked({newPixelScrollX})
  | HorizontalScrollbarAfterTrackClicked({newPixelScrollX})
  | HorizontalScrollbarMouseDrag({newPixelScrollX}) => (
      Editor.scrollToPixelX(~pixelX=newPixelScrollX, editor),
      Nothing,
    )
  | HorizontalScrollbarMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelX(
        ~pixelX=deltaWheel *. Constants.scrollbarWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | HorizontalScrollbarMouseDown
  | HorizontalScrollbarMouseRelease
  | VerticalScrollbarMouseRelease
  | VerticalScrollbarMouseDown => (editor, Nothing)
  | MouseHovered({location}) => (editor, MouseHovered(location))
  | MouseMoved({location}) => (editor, MouseMoved(location))
  | SelectionChanged(selection) => (
      Editor.setSelection(~selection, editor),
      Nothing,
    )
  | CursorsChanged(cursors) => (
      Editor.setVimCursors(~cursors, editor),
      Nothing,
    )
  | ScrollToLine(line) => (Editor.scrollToLine(~line, editor), Nothing)
  | ScrollToColumn(column) => (
      Editor.scrollToColumn(~column, editor),
      Nothing,
    )
  };
};
