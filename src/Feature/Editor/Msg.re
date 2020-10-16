open EditorCoreTypes;
open Oni_Core;

[@deriving show({with_path: false})]
type t =
  | VerticalScrollbarBeforeTrackClicked({newPixelScrollY: float})
  | VerticalScrollbarAfterTrackClicked({newPixelScrollY: float})
  | VerticalScrollbarMouseDown
  | VerticalScrollbarMouseDrag({newPixelScrollY: float})
  | VerticalScrollbarMouseRelease
  | VerticalScrollbarMouseWheel({deltaWheel: float})
  | HorizontalScrollbarBeforeTrackClicked({newPixelScrollX: float})
  | HorizontalScrollbarAfterTrackClicked({newPixelScrollX: float})
  | HorizontalScrollbarMouseDown
  | HorizontalScrollbarMouseDrag({newPixelScrollX: float})
  | HorizontalScrollbarMouseRelease
  | HorizontalScrollbarMouseWheel({deltaWheel: float})
  | MinimapMouseWheel({deltaWheel: float})
  | MinimapClicked({viewLine: int})
  | MinimapDragged({newPixelScrollY: float})
  | EditorMouseWheel({
      deltaX: float,
      deltaY: float,
      shiftKey: bool,
    })
  | MouseHovered({bytePosition: BytePosition.t})
  | MouseMoved({bytePosition: BytePosition.t})
  | SelectionChanged([@opaque] VisualRange.t)
  | SelectionCleared
  | ModeChanged([@opaque] Vim.Mode.t)
  | ScrollToLine(int)
  | ScrollToColumn(int)
  | MinimapEnabledConfigChanged(bool)
  | LineHeightConfigChanged(LineHeight.t);
