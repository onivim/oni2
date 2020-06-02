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
type msg =
  | VerticalScrollbarMouseDown({ newPixelScrollY: float })
  | VerticalScrollbarMouseDrag({ newPixelScrollY: float })
  | VerticalScrollbarMouseRelease

type outmsg = 
| Nothing;

type model = Editor.t;

let update = (msg) =>
  switch (msg) {
  | VerticalScrollbarMouseRelease
  | VerticalScrollbarMouseDrag(_)
  | VerticalScrollbarMouseDown(_) => Nothing
  };

