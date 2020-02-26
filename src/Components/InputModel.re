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

  (textSlice, Selection.collapseRelative(~text=textSlice, ~selection, Left(1)));
};

let removeSelection = (text, selection) => {
  let (textSlice, focus) = removeAfter(
    Selection.rangeStart(selection),
    text,
    ~count=Selection.range(selection)
  );

  (textSlice, Selection.collapse(~text=textSlice, focus));
};


let removeCharAfter = (text, selection) => {
  let (textSlice, focus) = removeAfter(Selection.focus(selection), text);

  (textSlice, Selection.collapse(~text=textSlice, focus));
};


let collapseWord = (text, selection, direction) => {
  let newSelection = Selection.focus(selection)
    |> direction(text)
    |> Selection.collapse(~text);

  (text, newSelection);
}

let extendWord = (text, selection, direction) => {
  let newSelection = Selection.focus(selection)
    |> direction(text)
    |> Selection.extend(~text, ~selection);

  (text, newSelection);
}

let addCharacter = (key, text, selection) => {
  let (newText, focus) = add(~at=Selection.focus(selection), key, text);

  (newText, Selection.collapse(~text=newText, focus));
};


let replacesSelection = (key, text, selection) => {
  let (textSlice, sliceSelection) =
    removeSelection(text, selection);
  let (newText, focus) = add(~at=Selection.focus(sliceSelection), key, textSlice);

  (newText, Selection.collapse(~text=newText, focus));
};

let handleInput = (~text, ~selection, key) => {
  switch (key, Selection.isCollapsed(selection)) {
    | ("<LEFT>", true)   => (text, Selection.collapseRelative(~text, ~selection, Left(1)));
    | ("<LEFT>", false)  => (text, Selection.collapseRelative(~text, ~selection, Left(0)));
    | ("<RIGHT>", true)  => (text, Selection.collapseRelative(~text, ~selection, Right(1)));
    | ("<RIGHT>", false) => (text, Selection.collapseRelative(~text, ~selection, Right(0)));
    | ("<BS>", true) => removeCharBefore(text, selection);
    | ("<BS>", false) => removeSelection(text, selection);
    | ("<DEL>", true) => removeCharAfter(text, selection);
    | ("<DEL>", false) => removeSelection(text, selection);
    | ("<HOME>", _) => (text, Selection.collapse(~text, 0));
    | ("<END>", _) => (text, Selection.collapse(~text, String.length(text)));
    | ("<S-LEFT>", _) => (text, Selection.extendRelative(~text, ~selection, Left(1)));
    | ("<S-RIGHT>", _) => (text, Selection.extendRelative(~text, ~selection, Right(1)));
    | ("<C-LEFT>", _) => collapseWord(text, selection, findPrevWordBoundary);
    | ("<C-RIGHT>", _) => collapseWord(text, selection, findNextWordBoundary);
    | ("<S-HOME>", _) => (text, Selection.extend(~text, ~selection, 0));
    | ("<S-END>", _) => (text, Selection.extend(~text, ~selection, String.length(text)));
    | ("<S-C-LEFT>", _) => extendWord(text, selection, findPrevWordBoundary);
    | ("<S-C-RIGHT>", _) => extendWord(text, selection, findNextWordBoundary);
    | ("<C-a>", _) => (text, Selection.create(~text, ~anchor=0, ~focus=String.length(text)))
    | (key, true) when String.length(key) == 1 => addCharacter(key, text, selection);
    | (key, false) when String.length(key) == 1 => replacesSelection(key, text, selection);
    | (_, _) => (text, selection);
  };
};