open Oni_Core;
open Oni_Core.Types;

open Actions;

type t = {editorRanges: list(Range.t)};

let default: t = {editorRanges: []};

let getVisibleRangesForEditor = (editor: Editor.t, metrics: EditorMetrics.t) => {
  let topVisibleLine = Editor.getTopVisibleLine(editor, metrics);
  let bottomVisibleLine = Editor.getBottomVisibleLine(editor, metrics);

  let leftVisibleColumn = Editor.getLeftVisibleColumn(editor, metrics);

  let (_, bufferWidthInCharacters) =
    Editor.getLinesAndColumns(editor, metrics);

  let i = ref(topVisibleLine);
  let eRanges = ref([]);

  while (i^ < bottomVisibleLine) {
    let idx = i^;
    let range =
      Range.ofInt0(
        ~startLine=idx,
        ~startCharacter=leftVisibleColumn,
        ~endLine=idx,
        ~endCharacter=leftVisibleColumn + bufferWidthInCharacters,
        (),
      );

    eRanges := [range, ...eRanges^];
    incr(i);
  };

  {editorRanges: eRanges^};
};

let getVisibleRangesForBuffer = (bufferId: int, state: State.t) => {
  let editors =
    WindowTree.getSplits(state.windowManager.windowTree)
    |> List.map((split: WindowTree.split) => split.editorGroupId)
    |> Utility.filterMap(
         EditorGroups.getEditorGroupById(state.editorGroups),
       )
    |> Utility.filterMap(eg =>
         switch (EditorGroup.getActiveEditor(eg)) {
         | None => None
         | Some(v) =>
           let tup = (eg.metrics, v);
           Some(tup);
         }
       );

  let flatten = (prev: t, curr: t) => {
    {editorRanges: List.append(prev.editorRanges, curr.editorRanges)};
  };

  editors
  |> List.map(((metrics, editor)) =>
       getVisibleRangesForEditor(editor, metrics)
     )
  |> List.fold_left(flatten, default);
};
