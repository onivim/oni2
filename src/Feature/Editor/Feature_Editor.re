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
