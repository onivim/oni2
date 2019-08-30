open Oni_Core;

open Actions;

type minimapVisibleRanges = {
  topLine: int,
  bottomLine: int,
  width: int,
};

type individualRange = {
  editorRanges: list(Range.t),
  minimap: minimapVisibleRanges,
};

type t = {
  editorRanges: list(list(Range.t)),
  minimaps: list(minimapVisibleRanges),
};

let default: t = {editorRanges: [], minimaps: []};

let getVisibleRangesForEditor = (editor: Editor.t, metrics: EditorMetrics.t) => {
  let topVisibleLine = Editor.getTopVisibleLine(editor, metrics);
  let bottomVisibleLine = Editor.getBottomVisibleLine(editor, metrics);

  let leftVisibleColumn = Editor.getLeftVisibleColumn(editor, metrics);

  let {bufferWidthInCharacters, minimapWidthInCharacters, _}: EditorLayout.t =
    Editor.getLayout(editor, metrics);

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

  let minimapLineHeight =
    Constants.default.minimapCharacterHeight
    + Constants.default.minimapLineSpacing;

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
  let minimap = {
    topLine: minimapTopLine,
    bottomLine: minimapBottomLine,
    width: minimapWidthInCharacters,
  };

  {editorRanges: eRanges^, minimap};
};

let getVisibleBuffers = (state: State.t) => {
  WindowTree.getSplits(state.windowManager.windowTree)
  |> List.map((split: WindowTree.split) => split.editorGroupId)
  |> Utility.filterMap(EditorGroups.getEditorGroupById(state.editorGroups))
  |> Utility.filterMap(EditorGroup.getActiveEditor)
  |> List.map(e => e.bufferId);
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
       )
    |> List.filter(((_, editor)) => editor.bufferId == bufferId);

  let flatten = (prev: t, curr: individualRange) => {
    {
      editorRanges: [curr.editorRanges, ...prev.editorRanges],
      minimaps: [curr.minimap, ...prev.minimaps],
    };
  };

  editors
  |> List.map(((metrics, editor)) =>
       getVisibleRangesForEditor(editor, metrics)
     )
  |> List.fold_left(flatten, default);
};
