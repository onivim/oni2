open Oni_Core;
open Utility;

let wordSeparators = " ./\\()\"'-:,.;<>~!@#$%^&*|+=[]{}`~?";

let separatorOnIndex = (text, index) => {
  String.contains(wordSeparators, String.get(text, index));
}

let findNextWordBoundary = (text, cursorPosition) => {
  let finalIndex = String.length(text);
  let index = ref(min(cursorPosition + 1, finalIndex));

  while (index^ < finalIndex && !separatorOnIndex(text, index^)) {
    index:= index^ + 1;
  };

  index^;
};

let findPrevWordBoundary = (text, cursorPosition) => {
  let finalIndex = 0;
  let index = ref(max(cursorPosition - 1, finalIndex));

  while (index^ > finalIndex && !separatorOnIndex(text, index^ - 1)) {
    index:= index^ - 1;
  };

  index^;
};

let slice = (~start=0, ~stop=?, str) => {
  let length = String.length(str);
  let start = IntEx.clamp(~lo=0, ~hi=length, start);
  let stop =
    switch (stop) {
    | Some(index) => IntEx.clamp(~lo=0, ~hi=length, index)
    | None => length
    };

  String.sub(str, start, stop - start);
};

let removeBefore = (~count=1, index, text) => (
  slice(text, ~stop=index - count) ++ slice(text, ~start=index),
  max(0, index - count)
);

let removeAfter = (~count=1, index, text) => (
  slice(text, ~stop=index) ++ slice(text, ~start=index + count),
  index
);


let add = (~at as index, insert, text) => (
  slice(text, ~stop=index) ++ insert ++ slice(text, ~start=index),
  index + String.length(insert)
);

let moveCursorLeft = (text, cursorPosition, _) => {
  let leastCursorPoition = max(0, cursorPosition - 1);

  (text, leastCursorPoition, leastCursorPoition);
};

let moveCursorToSelectionBeginning = (text, cursorPosition, selectionPosition) => {
  let selectionBeginning = min(cursorPosition, selectionPosition);

  (text, selectionBeginning, selectionBeginning);
};

let moveSelectionLeft = (text, cursorPosition, selectionPosition) => (
  text,
  max(0, cursorPosition - 1),
  selectionPosition
);

let moveCursorRight = (text, cursorPosition, _) => {
  let mostCursorPosition = min(String.length(text), cursorPosition + 1);

  (text, mostCursorPosition, mostCursorPosition);
};

let moveCursorToSelectionEnding = (text, cursorPosition, selectionPosition) => {
  let selectionEnding = max(cursorPosition, selectionPosition);

  (text, selectionEnding, selectionEnding);
};

let moveSelectionRight = (text, cursorPosition, selectionPosition) => (
  text,
  min(String.length(text), cursorPosition + 1),
  selectionPosition
);

let removeCharBefore = (text, cursorPosition, _) => {
  let (textSlice, newCursorPosition) = removeBefore(cursorPosition, text);

  (textSlice, newCursorPosition, newCursorPosition);
};

let removeCharAfter = (text, cursorPosition, _) => {
  let (textSlice, _) = removeAfter(cursorPosition, text);

  (textSlice, cursorPosition, cursorPosition);
};

let removeSelection = (text, cursorPosition, selectionPosition) => {
  let leastPosition = min(cursorPosition, selectionPosition);
  let count = abs(cursorPosition - selectionPosition);

  let (textSlice, _) = removeAfter(leastPosition, text, ~count=count);

  (textSlice, leastPosition, leastPosition);
};

let moveCursorHome = (text, _, _) => (text, 0, 0)

let moveCursorEnd = (text, _, _) => {
  let textLength = String.length(text);

  (text, textLength, textLength);
};

let moveSelectionHome = (text, _, selectionPosition) => (
  text, 0, selectionPosition
);

let moveSelectionEnd = (text, _, selectionPosition) => (
  text,
  String.length(text),
  selectionPosition
);

let addCharacter = (key, text, cursorPosition, _) => {
  let (newText, newCursorPosition) = add(~at=cursorPosition, key, text);

  (newText, newCursorPosition, newCursorPosition);
};

let replacesSelection = (key, text, cursorPosition, selectionPosition) => {
  let (textSlice, sliceCursorPosition, _) =
    removeSelection(text, cursorPosition, selectionPosition);
  let (newText, newCursorPosition) = add(~at=sliceCursorPosition, key, textSlice);

  (newText, newCursorPosition, newCursorPosition);
};

let doNothing = (text, cursorPosition, selectionPosition) => (
  text,
  cursorPosition,
  selectionPosition
)

let moveSelectionPreviousWord = (text, cursorPosition, selectionPosition) => {
  (text, findPrevWordBoundary(text, cursorPosition), selectionPosition)
}

let moveSelectionNextWord = (text, cursorPosition, selectionPosition) => {
  (text, findNextWordBoundary(text, cursorPosition), selectionPosition)
}

let handleInput = (~text, ~cursorPosition, ~selectionPosition, key) => {
  let handler =
    switch (key, cursorPosition == selectionPosition) {
      | ("<LEFT>", true) => moveCursorLeft
      | ("<LEFT>", false) => moveCursorToSelectionBeginning
      | ("<RIGHT>", true) => moveCursorRight
      | ("<RIGHT>", false) => moveCursorToSelectionEnding
      | ("<BS>", true) => removeCharBefore
      | ("<BS>", false) => removeSelection
      | ("<DEL>", true) => removeCharAfter
      | ("<DEL>", false) => removeSelection
      | ("<HOME>", _) => moveCursorHome
      | ("<END>", _) => moveCursorEnd
      | ("<S-LEFT>", _) => moveSelectionLeft
      | ("<S-RIGHT>", _) => moveSelectionRight
      | ("<S-HOME>", _) => moveSelectionHome
      | ("<S-END>", _) => moveSelectionEnd
      | ("<S-C-LEFT>", _) => moveSelectionPreviousWord
      | ("<S-C-RIGHT>", _) => moveSelectionNextWord
      | (key, true) when String.length(key) == 1 => addCharacter(key)
      | (key, false) when String.length(key) == 1 => replacesSelection(key)
      | (_, _) => doNothing;
    };

  handler(text, cursorPosition, selectionPosition);
};

let removeCharBeforeS = (text, selection) => {
  let (textSlice, newCursorPosition) = removeBefore(Selection.focus(selection), text);

  (textSlice, Selection.collapse(selection, Left(textSlice, 1)));
};

let removeSelectionS = (text, selection) => {
  let (textSlice, focus) = removeAfter(
    Selection.rangeStart(selection),
    text,
    ~count=Selection.range(selection)
  );

  (textSlice, Selection.collapse(selection, Position(textSlice, focus)));
};


let removeCharAfterS = (text, selection) => {
  let (textSlice, focus) = removeAfter(Selection.focus(selection), text);

  (textSlice, Selection.collapse(selection, Position(textSlice, focus)));
};


let previousWordS = (action, text, selection) => {
  let prevFocus = Selection.focus(selection) |> findPrevWordBoundary(text);

  (text, action(selection, Selection.Position(text, prevFocus)));
}

let nextWordS = (action,  text, selection) => {
  let nextFocus = Selection.focus(selection) |> findNextWordBoundary(text);

  (text, action(selection, Selection.Position(text, nextFocus)));
}


let addCharacterS = (key, text, selection) => {
  let (newText, focus) = add(~at=Selection.focus(selection), key, text);

  print_int(Selection.focus(selection));
  print_int(focus);

  let newSelection = Selection.Position(newText, focus) |> Selection.collapse(selection);

  (newText, newSelection);
};


let replacesSelectionS = (key, text, selection) => {
  let (textSlice, sliceSelection) =
    removeSelectionS(text, selection);
  let (newText, focus) = add(~at=Selection.focus(sliceSelection), key, textSlice);

  (newText, Selection.collapse(selection, Position(newText, focus)));
};

let handleInputS = (~text, ~selection, key) => {
  switch (key, Selection.isCollapsed(selection)) {
    | ("<LEFT>", true) =>  (text, Selection.collapse(selection, Left(text, 1)));
    | ("<LEFT>", false) => (text, Selection.collapse(selection, Left(text, 0)));
    | ("<RIGHT>", true) => (text, Selection.collapse(selection, Right(text, 1)));
    | ("<RIGHT>", false) => (text, Selection.collapse(selection, Right(text, 0)));
    | ("<BS>", true) => removeCharBeforeS(text, selection);
    | ("<BS>", false) => removeSelectionS(text, selection);
    | ("<DEL>", true) => removeCharAfterS(text, selection);
    | ("<DEL>", false) => removeSelectionS(text, selection);
    | ("<HOME>", _) => (text, Selection.collapse(selection, Start));
    | ("<END>", _) => (text, Selection.collapse(selection, End(text)));
    | ("<S-LEFT>", _) => (text, Selection.extend(selection, Left(text, 1)));
    | ("<S-RIGHT>", _) => (text, Selection.extend(selection, Right(text, 1)));
    | ("<C-LEFT>", _) => previousWordS(Selection.collapse, text, selection); // add tests
    | ("<C-RIGHT>", _) => nextWordS(Selection.collapse, text, selection); // add tests
    | ("<S-HOME>", _) => (text, Selection.extend(selection, Start));
    | ("<S-END>", _) => (text, Selection.extend(selection, End(text)));
    | ("<S-C-LEFT>", _) => previousWordS(Selection.extend, text, selection);
    | ("<S-C-RIGHT>", _) => nextWordS(Selection.extend, text, selection);
    | ("<C-a>", _) => (text, Selection.create(text, ~anchor=0, ~focus=String.length(text)))
    | (key, true) when String.length(key) == 1 => addCharacterS(key, text, selection);
    | (key, false) when String.length(key) == 1 => replacesSelectionS(key, text, selection);
    | (_, _) => (text, selection);
  };
};