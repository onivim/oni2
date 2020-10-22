open EditorCoreTypes;
open Oni_Core;

module Editor = Feature_Editor.Editor;
module EditorLayout = Feature_Editor.EditorLayout;

type individualRange = {
  editorRanges: list(Range.t),
  minimapRanges: list(Range.t),
};

let getVisibleRangesForEditor = (editor: Editor.t) => {
  let topVisibleLine =
    Editor.getTopVisibleBufferLine(editor)
    |> EditorCoreTypes.LineNumber.toZeroBased;
  let bottomVisibleLine =
    Editor.getBottomVisibleBufferLine(editor)
    |> EditorCoreTypes.LineNumber.toZeroBased;

  let leftVisibleColumn = Editor.getLeftVisibleColumn(editor);

  let {bufferWidthInCharacters, minimapWidthInCharacters, _}: EditorLayout.t =
    Editor.getLayout(editor);

  let i = ref(max(topVisibleLine, 0));
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
    int_of_float(
      Editor.minimapScrollY(editor) /. float_of_int(minimapLineHeight),
    );
  let pixelHeight = Editor.visiblePixelHeight(editor);
  let viewLines = Editor.totalViewLines(editor);
  let minimapVisibleLines =
    int_of_float(
      float_of_int(pixelHeight) /. float_of_int(minimapLineHeight) +. 0.5,
    );
  let minimapBottomLine =
    min(minimapTopLine + minimapVisibleLines, viewLines);

  let ranges = max(0, minimapBottomLine - minimapTopLine);

  let minimapRanges =
    List.init(ranges, i => i + minimapTopLine)
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
  Feature_Layout.visibleEditors(state.layout)
  |> List.map(editor => Editor.getBufferId(editor));
};

type t = list((int, list(Range.t)));

let getVisibleRangesForBuffer = (bufferId: int, state: State.t) => {
  let editors =
    Feature_Layout.visibleEditors(state.layout)
    |> List.filter(editor => Editor.getBufferId(editor) == bufferId);

  let flatten = (prev: list(list(Range.t)), curr: individualRange) => {
    [curr.editorRanges, curr.minimapRanges, ...prev];
  };

  editors
  |> List.map(getVisibleRangesForEditor)
  |> List.fold_left(flatten, [])
  |> List.flatten;
};

let getVisibleBuffersAndRanges: State.t => t =
  (state: State.t) => {
    let visibleBuffers = getVisibleBuffers(state);

    List.map(b => (b, getVisibleRangesForBuffer(b, state)), visibleBuffers);
  };
