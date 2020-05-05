open EditorCoreTypes;
open Oni_Core;
open Oni_Model;
open Oni_Store;
open Feature_Editor;

/* Create a state with some editor size */
let simpleState = {
  let state =
    State.initial(
      ~getUserSettings=() => Ok(Config.Settings.empty),
      ~contributedCommands=[],
      ~workingDirectory=Sys.getcwd(),
    );

  Reducer.reduce(
    state,
    Actions.EditorGroupSizeChanged({
      id: EditorGroups.activeGroupId(state.editorGroups),
      width: 3440,
      height: 1440,
    }),
  );
};

let defaultFont: Service_Font.font = {
  fontFile: Revery.Environment.executingDirectory ++ "FiraCode-Regular.ttf",
  fontSize: 10.,
  measuredWidth: 10.,
  measuredHeight: 10.,
  descenderHeight: 1.,
  smoothing: Revery.Font.Smoothing.default,
};

let simpleState =
  Reducer.reduce(
    simpleState,
    Actions.EditorFont(Service_Font.FontLoaded(defaultFont)),
  );

let simpleEditor =
  Editor.create(~font=defaultFont, ())
  |> Editor.setSize(~pixelWidth=3440, ~pixelHeight=1440);
let editorGroup =
  EditorGroups.getActiveEditorGroup(simpleState.editorGroups)
  |> Option.value(~default=EditorGroup.create());

let thousandLines =
  Array.make(1000, "This is a buffer with a thousand lines!");

let createUpdateAction = (oldBuffer: Buffer.t, update: BufferUpdate.t) => {
  let newBuffer = Buffer.update(oldBuffer, update);
  Actions.BufferUpdate({update, oldBuffer, newBuffer});
};

let thousandLineBuffer = Buffer.ofLines(thousandLines);

let thousandLineState =
  Reducer.reduce(
    simpleState,
    createUpdateAction(
      thousandLineBuffer,
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
    createUpdateAction(
      thousandLineBuffer,
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
    createUpdateAction(
      Buffer.ofLines([||]),
      BufferUpdate.create(
        ~startLine=Index.zero,
        ~endLine=Index.fromZeroBased(1),
        ~lines=hundredThousandLines,
        ~version=1,
        (),
      ),
    ),
  );
