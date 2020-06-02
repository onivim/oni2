
[@deriving show({with_path: false})]
type t =
  | VerticalScrollbarMouseDown({ newPixelScrollY: float })
  | VerticalScrollbarMouseDrag({ newPixelScrollY: float })
  | VerticalScrollbarMouseRelease

