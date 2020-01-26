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
