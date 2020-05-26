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
  | FilesDropped({paths: list(string)});

let update = (msg, openFileEffect, noopEffect) =>
  switch (msg) {
  | FilesDropped({paths}) =>
    Service_OS.Effect.statMultiple(paths, (path, stats) =>
      if (stats.st_kind == S_REG) {
        openFileEffect(path);
      } else {
        noopEffect;
      }
    )
  };
