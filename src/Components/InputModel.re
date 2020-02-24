open Oni_Core;
open Utility;

let wordSeparators = " ./\\()\"'-:,.;<>~!@#$%^&*|+=[]{}`~?";

let separatorOnIndex = (text, index) => {
  String.contains(wordSeparators, String.get(text, index));
}

let findNextWordBoundary = (text, focus) => {
  let finalIndex = String.length(text);
  let index = ref(min(focus + 1, finalIndex));

  while (index^ < finalIndex && !separatorOnIndex(text, index^)) {
    index:= index^ + 1;
  };

  index^;
};

let findPrevWordBoundary = (text, focus) => {
  let finalIndex = 0;
  let index = ref(max(focus - 1, finalIndex));

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

let removeCharBefore = (text, selection) => {
  let (textSlice, _) = removeBefore(Selection.focus(selection), text);

  (textSlice, Selection.collapse(selection, Left(textSlice, 1)));
};

let removeSelection = (text, selection) => {
  let (textSlice, focus) = removeAfter(
    Selection.rangeStart(selection),
    text,
    ~count=Selection.range(selection)
  );

  (textSlice, Selection.collapse(selection, Position(textSlice, focus)));
};


let removeCharAfter = (text, selection) => {
  let (textSlice, focus) = removeAfter(Selection.focus(selection), text);

  (textSlice, Selection.collapse(selection, Position(textSlice, focus)));
};


let previousWord = (action, text, selection) => {
  let prevFocus = Selection.focus(selection) |> findPrevWordBoundary(text);

  (text, action(selection, Selection.Position(text, prevFocus)));
}

let nextWord = (action,  text, selection) => {
  let nextFocus = Selection.focus(selection) |> findNextWordBoundary(text);

  (text, action(selection, Selection.Position(text, nextFocus)));
}


let addCharacter = (key, text, selection) => {
  let (newText, focus) = add(~at=Selection.focus(selection), key, text);

  print_int(Selection.focus(selection));
  print_int(focus);

  let newSelection = Selection.Position(newText, focus) |> Selection.collapse(selection);

  (newText, newSelection);
};


let replacesSelection = (key, text, selection) => {
  let (textSlice, sliceSelection) =
    removeSelection(text, selection);
  let (newText, focus) = add(~at=Selection.focus(sliceSelection), key, textSlice);

  (newText, Selection.collapse(selection, Position(newText, focus)));
};

let handleInput = (~text, ~selection, key) => {
  switch (key, Selection.isCollapsed(selection)) {
    | ("<LEFT>", true) =>  (text, Selection.collapse(selection, Left(text, 1)));
    | ("<LEFT>", false) => (text, Selection.collapse(selection, Left(text, 0)));
    | ("<RIGHT>", true) => (text, Selection.collapse(selection, Right(text, 1)));
    | ("<RIGHT>", false) => (text, Selection.collapse(selection, Right(text, 0)));
    | ("<BS>", true) => removeCharBefore(text, selection);
    | ("<BS>", false) => removeSelection(text, selection);
    | ("<DEL>", true) => removeCharAfter(text, selection);
    | ("<DEL>", false) => removeSelection(text, selection);
    | ("<HOME>", _) => (text, Selection.collapse(selection, Start));
    | ("<END>", _) => (text, Selection.collapse(selection, End(text)));
    | ("<S-LEFT>", _) => (text, Selection.extend(selection, Left(text, 1)));
    | ("<S-RIGHT>", _) => (text, Selection.extend(selection, Right(text, 1)));
    | ("<C-LEFT>", _) => previousWord(Selection.collapse, text, selection);
    | ("<C-RIGHT>", _) => nextWord(Selection.collapse, text, selection);
    | ("<S-HOME>", _) => (text, Selection.extend(selection, Start));
    | ("<S-END>", _) => (text, Selection.extend(selection, End(text)));
    | ("<S-C-LEFT>", _) => previousWord(Selection.extend, text, selection);
    | ("<S-C-RIGHT>", _) => nextWord(Selection.extend, text, selection);
    | ("<C-a>", _) => (text, Selection.create(text, ~anchor=0, ~focus=String.length(text)))
    | (key, true) when String.length(key) == 1 => addCharacter(key, text, selection);
    | (key, false) when String.length(key) == 1 => replacesSelection(key, text, selection);
    | (_, _) => (text, selection);
  };
};