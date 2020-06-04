module BufferLineColorizer = BufferLineColorizer;
module BufferViewTokenizer = BufferViewTokenizer;
module Selection = Selection;

module Editor = Editor;
module EditorBuffer = EditorBuffer;
module EditorId = EditorId;
module EditorLayout = EditorLayout;
module EditorSurface = EditorSurface;

module EditorDiffMarkers = EditorDiffMarkers;

module Contributions = {
  let configuration = EditorConfiguration.contributions;
};

[@deriving show({with_path: false})]
type msg = Msg.t;

type outmsg =
  | Nothing;

type model = Editor.t;

module Constants = {
  let editorWheelMultiplier = 50.;
  let minimapWheelMultiplier = 150.;
  let scrollbarWheelMultiplier = 300.;
};

let update = (editor, msg) => {
  switch (msg) {
  | Msg.VerticalScrollbarAfterTrackClicked({newPixelScrollY})
  | Msg.VerticalScrollbarBeforeTrackClicked({newPixelScrollY})
  | Msg.VerticalScrollbarMouseDrag({newPixelScrollY}) => (
      Editor.scrollToPixelY(~pixelY=newPixelScrollY, editor),
      Nothing,
    )
  | Msg.MinimapMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelY(
        ~pixelY=deltaWheel *. Constants.minimapWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | Msg.MinimapClicked({viewLine}) => (
      Editor.scrollToLine(~line=viewLine, editor),
      Nothing,
    )
  | Msg.MinimapDragged({newPixelScrollY}) => (
      Editor.scrollToPixelY(~pixelY=newPixelScrollY, editor),
      Nothing,
    )
  | Msg.EditorMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelY(
        ~pixelY=deltaWheel *. Constants.editorWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | Msg.VerticalScrollbarMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelY(
        ~pixelY=deltaWheel *. Constants.scrollbarWheelMultiplier,
        editor,
      ),
      Nothing,
    )

  | Msg.VerticalScrollbarMouseRelease
  | Msg.VerticalScrollbarMouseDown => (editor, Nothing)
  };
};
