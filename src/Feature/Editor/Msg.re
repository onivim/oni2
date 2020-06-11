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
  | EditorMouseWheel({deltaWheel: float})
  | MouseHovered({location: EditorCoreTypes.Location.t})
  | MouseMoved({location: EditorCoreTypes.Location.t});
