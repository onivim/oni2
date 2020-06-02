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

let update = (editor, msg) => {
  let outmsg = switch (msg) {
  | Msg.VerticalScrollbarMouseRelease
  | Msg.VerticalScrollbarMouseDrag(_)
  | Msg.VerticalScrollbarMouseDown(_) => Nothing
  };
  (editor, outmsg);
};
