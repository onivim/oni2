open Oni_Core;
open Utility;

[@deriving show]
type msg =
  | Clicked({selection: Selection.t});

[@deriving show]
type t = {
  value: string,
  selection: Selection.t,
  placeholder: string,
};

let create = (~placeholder) => {
  value: "",
  selection: Selection.initial,
  placeholder,
};

let value = ({value, _}) => value;

let update = (msg, model) =>
  switch (msg) {
  | Clicked({selection}) => {...model, selection}
  };

module Internal = {
  let wordSeparators = " ./\\()\"'-:,.;<>~!@#$%^&*|+=[]{}`~?";

  let separatorOnIndexExn = (index, text) => {
    String.contains(wordSeparators, text.[index]);
  };

  let findNextWordBoundary = (text, focus) => {
    let finalIndex = String.length(text);
    let index = ref(min(focus + 1, finalIndex));

    while (index^ < finalIndex && !separatorOnIndexExn(index^, text)) {
      index := index^ + 1;
    };

    index^;
  };

  let findPrevWordBoundary = (text, focus) => {
    let finalIndex = 0;
    let index = ref(max(focus - 1, finalIndex));

    while (index^ > finalIndex && !separatorOnIndexExn(index^ - 1, text)) {
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

  let removeCharBefore = (text, selection: Selection.t) => {
    let (textSlice, _) = removeBefore(selection.focus, text);

    (
      textSlice,
      Selection.offsetLeft(selection)
      - 1
      |> Selection.collapsed(~text=textSlice),
    );
  };

  let removeWord = (text, selection: Selection.t) => {
    let lastWordStart = findPrevWordBoundary(text, selection.focus);
    let index = selection.focus;

    let count = index - lastWordStart;

    let (textSlice, idx) = removeBefore(~count, index, text);

    (textSlice, Selection.collapsed(~text=textSlice, idx));
  };

  let removeSelection = (text, selection) => {
    let (textSlice, focus) =
      removeAfter(
        Selection.offsetLeft(selection),
        text,
        ~count=Selection.length(selection),
      );

    (textSlice, Selection.collapsed(~text=textSlice, focus));
  };

  let removeCharAfter = (text, selection: Selection.t) => {
    let (textSlice, focus) = removeAfter(selection.focus, text);

    (textSlice, Selection.collapsed(~text=textSlice, focus));
  };

  let collapsePrevWord = (text, selection: Selection.t) => {
    let newSelection =
      selection.focus
      |> findPrevWordBoundary(text)
      |> Selection.collapsed(~text);

    (text, newSelection);
  };

  let collapseNextWord = (text, selection: Selection.t) => {
    let newSelection =
      selection.focus
      |> findNextWordBoundary(text)
      |> Selection.collapsed(~text);

    (text, newSelection);
  };

  let extendPrevWord = (text, selection: Selection.t) => {
    let newSelection =
      selection.focus
      |> findPrevWordBoundary(text)
      |> Selection.extend(~text, ~selection);

    (text, newSelection);
  };

  let extendNextWord = (text, selection: Selection.t) => {
    let newSelection =
      selection.focus
      |> findNextWordBoundary(text)
      |> Selection.extend(~text, ~selection);

    (text, newSelection);
  };

  let addCharacter = (key, text, selection: Selection.t) => {
    let (newText, focus) = add(~at=selection.focus, key, text);

    (newText, Selection.collapsed(~text=newText, focus));
  };

  let replacesSelection = (key, text, selection: Selection.t) => {
    let (textSlice, selectionSlice) = removeSelection(text, selection);
    let (newText, focus) = add(~at=selectionSlice.focus, key, textSlice);

    (newText, Selection.collapsed(~text=newText, focus));
  };

  let handleInput = (~text, ~selection: Selection.t, key) => {
    switch (key, Selection.isCollapsed(selection)) {
    | ("<LEFT>", true) => (
        text,
        Selection.offsetLeft(selection) - 1 |> Selection.collapsed(~text),
      )
    | ("<LEFT>", false) => (
        text,
        Selection.offsetLeft(selection) |> Selection.collapsed(~text),
      )
    | ("<RIGHT>", true) => (
        text,
        Selection.offsetLeft(selection) + 1 |> Selection.collapsed(~text),
      )
    | ("<RIGHT>", false) => (
        text,
        Selection.offsetRight(selection) |> Selection.collapsed(~text),
      )
    | ("<BS>", true) => removeCharBefore(text, selection)
    | ("<BS>", false) => removeSelection(text, selection)
    | ("<C-h>", true) => removeCharBefore(text, selection)
    | ("<C-h>", false) => removeSelection(text, selection)
    | ("<C-w>", true) => removeWord(text, selection)
    | ("<C-w>", false) => removeSelection(text, selection)
    | ("<DEL>", true) => removeCharAfter(text, selection)
    | ("<DEL>", false) => removeSelection(text, selection)
    | ("<HOME>", _) => (text, Selection.collapsed(~text, 0))
    | ("<END>", _) => (
        text,
        Selection.collapsed(~text, String.length(text)),
      )
    | ("<S-LEFT>", _) => (
        text,
        selection.focus - 1 |> Selection.extend(~text, ~selection),
      )
    | ("<S-RIGHT>", _) => (
        text,
        selection.focus + 1 |> Selection.extend(~text, ~selection),
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
};

let handleInput = (~key, model) => {
  let (value, selection) =
    Internal.handleInput(~text=model.value, ~selection=model.selection, key);

  {...model, value, selection};
};
