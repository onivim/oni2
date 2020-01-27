open EditorCoreTypes;
open Oni_Core;

module Editor = Feature_Editor.Editor;
module EditorMetrics = Feature_Editor.EditorMetrics;
module EditorLayout = Feature_Editor.EditorLayout;

type individualRange = {
  editorRanges: list(Range.t),
  minimapRanges: list(Range.t),
};

type t = {ranges: list(Range.t)};

let getVisibleRangesForEditor = (editor: Editor.t, metrics: EditorMetrics.t) => {
  let topVisibleLine = Editor.getTopVisibleLine(editor, metrics);
  let bottomVisibleLine = Editor.getBottomVisibleLine(editor, metrics);

  let leftVisibleColumn = Editor.getLeftVisibleColumn(editor, metrics);

  let {bufferWidthInCharacters, minimapWidthInCharacters, _}: EditorLayout.t =
    Editor.getLayout(editor, metrics);

  let i = ref(max(topVisibleLine - 1, 0));
  let eRanges = ref([]);

  while (i^ <= bottomVisibleLine) {
    let idx = i^;
    let range =
      Range.{
        start:
          Location.{
            line: Index.fromZeroBased(idx),
            column: Index.fromZeroBased(leftVisibleColumn),
          },
        stop:
          Location.{
            line: Index.fromZeroBased(idx),
            column:
              Index.fromZeroBased(
                leftVisibleColumn + bufferWidthInCharacters,
              ),
          },
      };

    eRanges := [range, ...eRanges^];
    incr(i);
  };

  let minimapLineHeight =
    Constants.minimapCharacterHeight + Constants.minimapLineSpacing;

  let minimapTopLine =
    int_of_float(editor.minimapScrollY /. float_of_int(minimapLineHeight));
  let minimapVisibleLines =
    int_of_float(
      float_of_int(metrics.pixelHeight)
      /. float_of_int(minimapLineHeight)
      +. 0.5,
    );
  let minimapBottomLine =
    min(minimapTopLine + minimapVisibleLines, editor.viewLines);

  let minimapRanges =
    List.init(minimapBottomLine - minimapTopLine, i => i + minimapTopLine)
    |> List.map(i =>
         Range.{
           start:
             Location.{line: Index.fromZeroBased(i), column: Index.zero},
           stop:
             Location.{
               line: Index.fromZeroBased(i),
               column: Index.fromZeroBased(minimapWidthInCharacters + 1),
             },
         }
       );

  {editorRanges: eRanges^, minimapRanges};
};

let getVisibleBuffers = (state: State.t) => {
  WindowTree.getSplits(state.windowManager.windowTree)
  |> List.map((split: WindowTree.split) => split.editorGroupId)
  |> List.filter_map(EditorGroups.getEditorGroupById(state.editorGroups))
  |> List.filter_map(EditorGroup.getActiveEditor)
  |> List.map(e => e.Editor.bufferId);
};

let getVisibleRangesForBuffer = (bufferId: int, state: State.t) => {
  let editors =
    WindowTree.getSplits(state.windowManager.windowTree)
    |> List.map((split: WindowTree.split) => split.editorGroupId)
    |> List.filter_map(EditorGroups.getEditorGroupById(state.editorGroups))
    |> List.filter_map(eg =>
         switch (EditorGroup.getActiveEditor(eg)) {
         | None => None
         | Some(v) =>
           let tup = (eg.metrics, v);
           Some(tup);
         }
       )
    |> List.filter(((_, editor)) => editor.Editor.bufferId == bufferId);

  let flatten = (prev: list(list(Range.t)), curr: individualRange) => {
    [curr.editorRanges, curr.minimapRanges, ...prev];
  };

  editors
  |> List.map(((metrics, editor)) =>
       getVisibleRangesForEditor(editor, metrics)
     )
  |> List.fold_left(flatten, [])
  |> List.flatten;
};

let getVisibleBuffersAndRanges = (state: State.t) => {
  let visibleBuffers = getVisibleBuffers(state);

  List.map(b => (b, getVisibleRangesForBuffer(b, state)), visibleBuffers);
};
