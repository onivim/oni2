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
  | EditorMouseEnter
  | EditorMouseDown({
      time: [@opaque] Revery.Time.t,
      pixelX: float,
      pixelY: float,
    })
  | EditorMouseMoved({
      time: [@opaque] Revery.Time.t,
      pixelX: float,
      pixelY: float,
    })
  | EditorMouseUp({
      time: [@opaque] Revery.Time.t,
      pixelX: float,
      pixelY: float,
    })
  | EditorMouseLeave
  | MouseHovered
  //  | MouseMoved({bytePosition: BytePosition.t})
  | ModeChanged({
      mode: [@opaque] Vim.Mode.t,
      effects: [@opaque] list(Vim.Effect.t),
    })
  | InlineElementSizeChanged({
      key: string,
      uniqueId: string,
      height: int,
    })
  | Internal(Editor.msg);
