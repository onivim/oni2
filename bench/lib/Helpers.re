open EditorCoreTypes;
open Oni_Core;
open Oni_Model;
open Oni_Store;
open Feature_Editor;

let metrics =
  EditorMetrics.{
    pixelWidth: 1600,
    pixelHeight: 1200,
    lineHeight: 10.,
    characterWidth: 10.,
  };

/* Create a state with some editor size */
let simpleState = {
  let state = State.create();

  Reducer.reduce(
    state,
    Actions.EditorGroupSetSize(
      EditorGroups.activeGroupId(state.editorGroups),
      EditorSize.create(~pixelWidth=3440, ~pixelHeight=1440, ()),
    ),
  );
};

let simpleState =
  Reducer.reduce(
    simpleState,
    Actions.SetEditorFont(
      EditorFont.create(
        ~fontFile=
          Revery.Environment.executingDirectory ++ "FiraCode-Regular.ttf",
        ~fontSize=10.,
        ~measuredWidth=10.,
        ~measuredHeight=10.,
        ~descenderHeight=1.,
        (),
      ),
    ),
  );

let simpleEditor = Editor.create();
let editorGroup =
  EditorGroups.getActiveEditorGroup(simpleState.editorGroups)
  |> Option.value(~default=EditorGroup.create());

let thousandLines =
  Array.make(1000, "This is a buffer with a thousand lines!");

let thousandLineBuffer = Buffer.ofLines(thousandLines);

let thousandLineState =
  Reducer.reduce(
    simpleState,
    Actions.BufferUpdate(
      BufferUpdate.create(
        ~startLine=Index.zero,
        ~endLine=Index.fromZeroBased(1),
        ~lines=thousandLines,
        ~version=1,
        (),
      ),
    ),
  );

/* Apply indents so we go into deeper and deeper indentation, before returning back. */
let applyIndent = (line, lineNum, fileLen) =>
  if (lineNum < fileLen / 2) {
    String.make(lineNum, '\t') ++ line;
  } else {
    String.make(fileLen - lineNum, '\t') ++ line;
  };

let thousandLinesWithIndents =
  Array.mapi((i, l) => applyIndent(l, i, 1000), thousandLines);

let thousandLineStateWithIndents =
  Reducer.reduce(
    simpleState,
    Actions.BufferUpdate(
      BufferUpdate.create(
        ~startLine=Index.zero,
        ~endLine=Index.fromZeroBased(1),
        ~lines=thousandLinesWithIndents,
        ~version=1,
        (),
      ),
    ),
  );

let hundredThousandLines =
  Array.make(100000, "This is a buffer with a hundred thousand lines!");

let hundredThousandLineState =
  Reducer.reduce(
    simpleState,
    Actions.BufferUpdate(
      BufferUpdate.create(
        ~startLine=Index.zero,
        ~endLine=Index.fromZeroBased(1),
        ~lines=hundredThousandLines,
        ~version=1,
        (),
      ),
    ),
  );
