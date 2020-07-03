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
    })
  | MouseHovered({location: Location.t})
  | MouseMoved({location: Location.t})
  | SelectionChanged([@opaque] VisualRange.t)
  | CursorsChanged([@opaque] list(Vim.Cursor.t))
  | MinimapEnabledConfigChanged(bool);
