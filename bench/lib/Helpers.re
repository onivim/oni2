open Oni_Core;
open Oni_Model;

/* Create a state with some editor size */
let simpleState =
  Reducer.reduce(
    State.create(),
    Actions.SetEditorSize(
      Types.EditorSize.create(~pixelWidth=1600, ~pixelHeight=1200, ()),
    ),
  );
let simpleState =
  Reducer.reduce(
    simpleState,
    Actions.SetEditorFont(
      Types.EditorFont.create(
        ~fontFile="dummy",
        ~fontSize=14,
        ~measuredWidth=14,
        ~measuredHeight=14,
        (),
      ),
    ),
  );

let thousandLines =
  Array.make(1000, "This is a buffer with a thousand lines!") |> Array.to_list;

let thousandLineState =
  Reducer.reduce(
    simpleState,
    Actions.BufferUpdate(
      Types.BufferUpdate.create(
        ~startLine=0,
        ~endLine=1,
        ~lines=thousandLines,
        ~version=1,
        (),
      ),
    ),
  );

let hundredThousandLines =
  Array.make(100000, "This is a buffer with a hundred thousand lines!")
  |> Array.to_list;
let hundredThousandLineState =
  Reducer.reduce(
    simpleState,
    Actions.BufferUpdate(
      Types.BufferUpdate.create(
        ~startLine=0,
        ~endLine=1,
        ~lines=hundredThousandLines,
        ~version=1,
        (),
      ),
    ),
  );
