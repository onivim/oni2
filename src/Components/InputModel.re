open Oni_Core;
open Utility;

let wordSeparators = " ./\\()\"'-:,.;<>~!@#$%^&*|+=[]{}`~?";

let separatorOnIndex = (index, text) => {
  String.contains(wordSeparators, text.[index]);
};

let findNextWordBoundary = (text, focus) => {
  let finalIndex = String.length(text);
  let index = ref(min(focus + 1, finalIndex));

  while (index^ < finalIndex && !separatorOnIndex(index^, text)) {
    index := index^ + 1;
  };

  index^;
};

let findPrevWordBoundary = (text, focus) => {
  let finalIndex = 0;
  let index = ref(max(focus - 1, finalIndex));

  while (index^ > finalIndex && !separatorOnIndex(index^ - 1, text)) {
    index := index^ - 1;
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
  max(0, index - count),
);

let removeAfter = (~count=1, index, text) => (
  slice(text, ~stop=index) ++ slice(text, ~start=index + count),
  index,
);

let add = (~at as index, insert, text) => (
  slice(text, ~stop=index) ++ insert ++ slice(text, ~start=index),
  index + String.length(insert),
);

let removeCharBefore = (text, selection) => {
  let (textSlice, _) = removeBefore(Selection.focus(selection), text);

  (
    textSlice,
    Selection.rangeStart(selection)
    - 1
    |> Selection.collapsed(~text=textSlice),
  );
};

let removeSelection = (text, selection) => {
  let (textSlice, focus) =
    removeAfter(
      Selection.rangeStart(selection),
      text,
      ~count=Selection.rangeWidth(selection),
    );

  (textSlice, Selection.collapsed(~text=textSlice, focus));
};

let removeCharAfter = (text, selection) => {
  let (textSlice, focus) = removeAfter(Selection.focus(selection), text);

  (textSlice, Selection.collapsed(~text=textSlice, focus));
};

let collapsePrevWord = (text, selection) => {
  let newSelection =
    Selection.focus(selection)
    |> findPrevWordBoundary(text)
    |> Selection.collapsed(~text);

  (text, newSelection);
};

let collapseNextWord = (text, selection) => {
  let newSelection =
    Selection.focus(selection)
    |> findNextWordBoundary(text)
    |> Selection.collapsed(~text);

  (text, newSelection);
};

let extendPrevWord = (text, selection) => {
  let newSelection =
    Selection.focus(selection)
    |> findPrevWordBoundary(text)
    |> Selection.extend(~text, ~selection);

  (text, newSelection);
};

let extendNextWord = (text, selection) => {
  let newSelection =
    Selection.focus(selection)
    |> findNextWordBoundary(text)
    |> Selection.extend(~text, ~selection);

  (text, newSelection);
};

let addCharacter = (key, text, selection) => {
  let (newText, focus) = add(~at=Selection.focus(selection), key, text);

  (newText, Selection.collapsed(~text=newText, focus));
};

let replacesSelection = (key, text, selection) => {
  let (textSlice, selectionSlice) = removeSelection(text, selection);
  let (newText, focus) =
    add(~at=Selection.focus(selectionSlice), key, textSlice);

  (newText, Selection.collapsed(~text=newText, focus));
};

let handleInput = (~text, ~selection, key) => {
  switch (key, Selection.isCollapsed(selection)) {
  | ("<LEFT>", true) => (
      text,
      Selection.rangeStart(selection) - 1 |> Selection.collapsed(~text),
    )
  | ("<LEFT>", false) => (
      text,
      Selection.rangeStart(selection) |> Selection.collapsed(~text),
    )
  | ("<RIGHT>", true) => (
      text,
      Selection.rangeStart(selection) + 1 |> Selection.collapsed(~text),
    )
  | ("<RIGHT>", false) => (
      text,
      Selection.rangeEnd(selection) |> Selection.collapsed(~text),
    )
  | ("<BS>", true) => removeCharBefore(text, selection)
  | ("<BS>", false) => removeSelection(text, selection)
  | ("<DEL>", true) => removeCharAfter(text, selection)
  | ("<DEL>", false) => removeSelection(text, selection)
  | ("<HOME>", _) => (text, Selection.collapsed(~text, 0))
  | ("<END>", _) => (text, Selection.collapsed(~text, String.length(text)))
  | ("<S-LEFT>", _) => (
      text,
      Selection.focus(selection) - 1 |> Selection.extend(~text, ~selection),
    )
  | ("<S-RIGHT>", _) => (
      text,
      Selection.focus(selection) + 1 |> Selection.extend(~text, ~selection),
    )
  | ("<C-LEFT>", _) => collapsePrevWord(text, selection)
  | ("<C-RIGHT>", _) => collapseNextWord(text, selection)
  | ("<S-HOME>", _) => (text, Selection.extend(~text, ~selection, 0))
  | ("<S-END>", _) => (
      text,
      Selection.extend(~text, ~selection, String.length(text)),
    )
  | ("<S-C-LEFT>", _) => extendPrevWord(text, selection)
  | ("<S-C-RIGHT>", _) => extendNextWord(text, selection)
  | ("<C-a>", _) => (
      text,
      Selection.create(~text, ~anchor=0, ~focus=String.length(text)),
    )
  | (key, true) when String.length(key) == 1 =>
    addCharacter(key, text, selection)
  | (key, false) when String.length(key) == 1 =>
    replacesSelection(key, text, selection)
  | (_, _) => (text, selection)
  };
};
