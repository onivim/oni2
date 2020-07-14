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

let empty = create(~placeholder="");

let isEmpty = ({value, _}) => value == "";

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

let paste = (~text, model) => {
  let (value', selection') =
    if (Selection.isCollapsed(model.selection)) {
      Internal.addCharacter(text, model.value, model.selection);
    } else {
      Internal.replacesSelection(text, model.value, model.selection);
    };

  {...model, value: value', selection: selection'};
};

let set = (~text, ~cursor, model) => {
  ...model,
  value: text,
  selection: Selection.collapsed(~text, cursor),
};

let isCursorAtEnd = ({value, selection, _}) => {
  Selection.isCollapsed(selection) && selection.focus == String.length(value);
};

let cursorPosition = ({selection, _}) => selection.focus;

// TESTS

let%test_module "Model" =
  (module
   {
     let testString = "Some interesting. Test. String. Isn't it? Maybe";
     let testStringLength = String.length(testString);

     let collapsed = (~text=testString, position) => {
       value: text,
       selection: Selection.create(~text, ~anchor=position, ~focus=position),
       placeholder: "",
     };

     let notCollapsed = (~text=testString, ~anchor, ~focus, ()) => {
       value: text,
       selection: Selection.create(~text, ~anchor, ~focus),
       placeholder: "",
     };

     let%test_module "paste" =
       (module
        {
          let pasteText = "hello from clipboard";
          let pasteTextLength = pasteText |> String.length;
          let%test "empty" = {
            collapsed(~text="", 0)
            |> paste(~text=pasteText)
            == collapsed(~text=pasteText, pasteTextLength);
          };
          let%test "into text, no selection" = {
            collapsed(~text="abc", 1)
            |> paste(~text=pasteText)
            == collapsed(~text="a" ++ pasteText ++ "bc", pasteTextLength + 1);
          };
          let%test "into text, selection" = {
            notCollapsed(~text="abc", ~anchor=3, ~focus=0, ())
            |> paste(~text=pasteText)
            == collapsed(~text=pasteText, pasteTextLength);
          };
        });

     let%test_module "handleInput" =
       (module
        {
          let%test_module "When LEFT with no selection" =
            (module
             {
               let key = "<LEFT>";
               let%test "Moves cursor left for 1 character" = {
                 collapsed(4) |> handleInput(~key) == collapsed(3);
               };

               let%test "Doesn't move cursor less than 0 position" = {
                 collapsed(0) |> handleInput(~key) == collapsed(0);
               };

               let%test "Doesn't move cursor for blank string" = {
                 collapsed(~text="", 0)
                 |> handleInput(~key) == collapsed(~text="", 0);
               };
             });

          let%test_module "When LEFT with selection" =
            (module
             {
               let key = "<LEFT>";
               let%test "Moves cursor to the beginning of seleciton when cursor comes first" = {
                 notCollapsed(~anchor=4, ~focus=2, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=2, ~focus=2, ());
               };

               let%test "Moves cursor to the beginning of selection when cursor at beginning" = {
                 notCollapsed(~anchor=2, ~focus=4, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=2, ~focus=2, ());
               };
             });
        });
     let%test_module "When RIGHT with no selection" =
       (module
        {
          let key = "<RIGHT>";
          let%test "Moves cursor right for 1 character" = {
            collapsed(4) |> handleInput(~key) == collapsed(5);
          };

          let%test "Doesn't move cursor position more than string length" = {
            collapsed(testStringLength)
            |> handleInput(~key) == collapsed(testStringLength);
          };

          let%test "Doesn't move cursor position for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
        });
     let%test_module "When RIGHT with  selection" =
       (module
        {
          let key = "<RIGHT>";
          let%test "Moves cursor to the end of selection when cursor comes first" = {
            notCollapsed(~anchor=2, ~focus=4, ())
            |> handleInput(~key) == notCollapsed(~anchor=4, ~focus=4, ());
          };

          let%test "Moves cursor to the end of selection when cursor comes last" = {
            notCollapsed(~anchor=4, ~focus=2, ())
            |> handleInput(~key) == notCollapsed(~anchor=4, ~focus=4, ());
          };

          let%test "Moves cursor to the end of selection when cursor at the beginning" = {
            notCollapsed(~anchor=testStringLength, ~focus=0, ())
            |> handleInput(~key)
            == notCollapsed(
                 ~anchor=testStringLength,
                 ~focus=testStringLength,
                 (),
               );
          };
        });
     let%test_module "When <C-w> with no selection" =
       (module
        {
          let key = "<C-w>";
          let%test "Removes word characters on the left of cursor" = {
            collapsed(4)
            |> handleInput(~key)
            == collapsed(
                 ~text=" interesting. Test. String. Isn't it? Maybe",
                 0,
               );
          };

          let%test "Doesn't remove anything if cursor at the beginning" = {
            collapsed(0) |> handleInput(~key) == collapsed(0);
          };

          let%test "Don't do anything for blank string" = {
            collapsed(~text="", -1)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
        });
     let%test_module "When <BS> with no selection" =
       (module
        {
          let keys = ["<BS>", "<C-h>"];
          let%test "Removes character on the left of cursor" = {
            keys
            |> List.for_all(key => {
                 collapsed(4)
                 |> handleInput(~key)
                 == collapsed(
                      ~text="Som interesting. Test. String. Isn't it? Maybe",
                      3,
                    )
               });
          };
          let%test "Doesn't remove character if cursor at the beginning" = {
            keys
            |> List.for_all(key => {
                 collapsed(0) |> handleInput(~key) == collapsed(0)
               });
          };
          let%test "Don't do anything for blank string" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text="", -1)
                 |> handleInput(~key) == collapsed(~text="", 0)
               });
          };
        });
     let%test_module "When <BS> with selection" =
       (module
        {
          let key = "<BS>";
          let%test "Removes selection when cursor comes first" = {
            notCollapsed(~anchor=4, ~focus=2, ())
            |> handleInput(~key)
            == notCollapsed(
                 ~anchor=2,
                 ~focus=2,
                 ~text="So interesting. Test. String. Isn't it? Maybe",
                 (),
               );
          };
          let%test "Removes selection when cursor comes last" = {
            notCollapsed(~anchor=2, ~focus=4, ())
            |> handleInput(~key)
            == notCollapsed(
                 ~anchor=2,
                 ~focus=2,
                 ~text="So interesting. Test. String. Isn't it? Maybe",
                 (),
               );
          };
        });
     let%test_module "When <DEL> with no selection" =
       (module
        {
          let key = "<DEL>";
          let%test "Removes character on the right side of cursor" = {
            collapsed(4)
            |> handleInput(~key)
            == collapsed(
                 ~text="Someinteresting. Test. String. Isn't it? Maybe",
                 4,
               );
          };
          let%test "Doesn't remove character if cursor at the end" = {
            collapsed(testStringLength)
            |> handleInput(~key) == collapsed(testStringLength);
          };
          let%test "Doesn't do anything for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
        });
     let%test_module "When <DEL> with selection" =
       (module
        {
          let key = "<DEL>";
          let%test "Removes selection when cursor comes first" = {
            notCollapsed(~anchor=4, ~focus=2, ())
            |> handleInput(~key)
            == notCollapsed(
                 ~anchor=2,
                 ~focus=2,
                 ~text="So interesting. Test. String. Isn't it? Maybe",
                 (),
               );
          };
          let%test "Removes selection when cursor comes last" = {
            notCollapsed(~anchor=2, ~focus=4, ())
            |> handleInput(~key)
            == notCollapsed(
                 ~anchor=2,
                 ~focus=2,
                 ~text="So interesting. Test. String. Isn't it? Maybe",
                 (),
               );
          };
        });
     let%test_module "When <HOME> with no selection" =
       (module
        {
          let key = "<HOME>";
          let%test "Moves cursor to the beginning" = {
            collapsed(4) |> handleInput(~key) == collapsed(0);
          };
          let%test "Doesn't move cursor if it's at the beginning" = {
            collapsed(0) |> handleInput(~key) == collapsed(0);
          };
          let%test "Doesn't move cursor position for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
        });
     let%test_module "When <HOME> with selection" =
       (module
        {
          let key = "<HOME>";
          let%test "Moves cursor to the beginning and discards selection" = {
            notCollapsed(~anchor=12, ~focus=5, ())
            |> handleInput(~key) == collapsed(0);
          };
          let%test "Doesn't move cursor if it's at the beginning and discard selection" = {
            notCollapsed(~anchor=8, ~focus=0, ())
            |> handleInput(~key) == collapsed(0);
          };
        });
     let%test_module "When <END> with no selection" =
       (module
        {
          let key = "<END>";
          let%test "Moves cursor to the end" = {
            collapsed(4) |> handleInput(~key) == collapsed(testStringLength);
          };
          let%test "Doesn't move cursor if its at the end" = {
            collapsed(testStringLength)
            |> handleInput(~key) == collapsed(testStringLength);
          };
          let%test "Doesn't move cursor for blank string" = {
            collapsed(~text="", testStringLength)
            |> handleInput(~key) == collapsed(~text="", testStringLength);
          };
        });
     let%test_module "When <END> with selection" =
       (module
        {
          let key = "<END>";
          let%test "Moves cursor to the end" = {
            notCollapsed(~anchor=11, ~focus=5, ())
            |> handleInput(~key)
            == notCollapsed(
                 ~anchor=testStringLength,
                 ~focus=testStringLength,
                 (),
               );
          };
          let%test "Doesn't move cursor if it's at the end and discard selection" = {
            notCollapsed(~anchor=testStringLength, ~focus=4, ())
            |> handleInput(~key)
            == notCollapsed(
                 ~anchor=testStringLength,
                 ~focus=testStringLength,
                 (),
               );
          };
        });
     let%test_module "When <S-LEFT>" =
       (module
        {
          let key = "<S-LEFT>";
          let%test "Moves cursor 1 character left and add selection, without selection" = {
            collapsed(4)
            |> handleInput(~key) == notCollapsed(~anchor=4, ~focus=3, ());
          };
          let%test "Moves cursor 1 character left and increase selection, with selection" = {
            notCollapsed(~anchor=11, ~focus=4, ())
            |> handleInput(~key) == notCollapsed(~anchor=11, ~focus=3, ());
          };
          let%test "Doesn't move cursor position when it is at the beginning" = {
            notCollapsed(~anchor=5, ~focus=0, ())
            |> handleInput(~key) == notCollapsed(~anchor=5, ~focus=0, ());
          };
          let%test "Doesn't move cursor when it is at the beginning and no selection" = {
            collapsed(0) |> handleInput(~key) == collapsed(0);
          };
          let%test "Doesn't move cursor for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
          let%test "Move cursor 1 character left and undo selection" = {
            notCollapsed(~anchor=4, ~focus=5, ())
            |> handleInput(~key) == collapsed(4);
          };
          let%test "Move cursor 1 character left and decrease seelction" = {
            notCollapsed(~anchor=4, ~focus=8, ())
            |> handleInput(~key) == notCollapsed(~anchor=4, ~focus=7, ());
          };
        });
     let%test_module "When <S-RIGHT>" =
       (module
        {
          let key = "<S-RIGHT>";
          let%test "Moves cursor to 1 character right and add selection" = {
            collapsed(4)
            |> handleInput(~key) == notCollapsed(~anchor=4, ~focus=5, ());
          };
          let%test "Moves cursor to 1 character right and increase selection" = {
            notCollapsed(~anchor=4, ~focus=11, ())
            |> handleInput(~key) == notCollapsed(~anchor=4, ~focus=12, ());
          };
          let%test "Doesn't move cursor position when at the end" = {
            notCollapsed(~anchor=5, ~focus=testStringLength, ())
            |> handleInput(~key)
            == notCollapsed(~anchor=5, ~focus=testStringLength, ());
          };
          let%test "Doesn't move cursor position when at the end and no selection" = {
            collapsed(testStringLength)
            |> handleInput(~key) == collapsed(testStringLength);
          };
          let%test "Doesn't move cursor position for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
          let%test "Moves cursor to 1 character right and undo selection" = {
            notCollapsed(~anchor=6, ~focus=5, ())
            |> handleInput(~key) == collapsed(6);
          };
          let%test "Moves cursor to 1 character right and decrease selection" = {
            notCollapsed(~anchor=8, ~focus=3, ())
            |> handleInput(~key) == notCollapsed(~anchor=8, ~focus=4, ());
          };
        });
     let%test_module "When <S-HOME>" =
       (module
        {
          let key = "<S-HOME>";
          let%test "Moves cursor to the beginning and add selection" = {
            collapsed(4)
            |> handleInput(~key) == notCollapsed(~anchor=4, ~focus=0, ());
          };
          let%test "Moves cursor to the beginning and increase selection" = {
            notCollapsed(~anchor=7, ~focus=4, ())
            |> handleInput(~key) == notCollapsed(~anchor=7, ~focus=0, ());
          };
          let%test "Doesn't move cursor position when at the beginning" = {
            notCollapsed(~anchor=5, ~focus=0, ())
            |> handleInput(~key) == notCollapsed(~anchor=5, ~focus=0, ());
          };
          let%test "Doesn't move cursor position when at beginning and no selection" = {
            collapsed(0) |> handleInput(~key) == collapsed(0);
          };
          let%test "Doesn't move cursor position for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
          let%test "Moves cursor to the beginning and undo selection" = {
            notCollapsed(~anchor=0, ~focus=6, ())
            |> handleInput(~key) == collapsed(0);
          };
          let%test "Moves cursor to the beginning and decrease selection" = {
            notCollapsed(~anchor=3, ~focus=8, ())
            |> handleInput(~key) == notCollapsed(~anchor=3, ~focus=0, ());
          };
        });
     let%test_module "When <S-END>" =
       (module
        {
          let key = "<S-END>";
          let%test "Moves cursor to the end and add selection" = {
            collapsed(4)
            |> handleInput(~key)
            == notCollapsed(~anchor=4, ~focus=testStringLength, ());
          };
          let%test "Moves cursor to the end and increase selection" = {
            notCollapsed(~anchor=4, ~focus=8, ())
            |> handleInput(~key)
            == notCollapsed(~anchor=4, ~focus=testStringLength, ());
          };
          let%test "Doesn't move cursor position when it is at the end" = {
            notCollapsed(~anchor=5, ~focus=testStringLength, ())
            |> handleInput(~key)
            == notCollapsed(~anchor=5, ~focus=testStringLength, ());
          };
          let%test "Doesn't move cursor position when it is at the end and no selection" = {
            collapsed(testStringLength)
            |> handleInput(~key) == collapsed(testStringLength);
          };
          let%test "Doesn't move cursor position for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
          let%test "Moves cursor to the end and undo selection" = {
            notCollapsed(~anchor=testStringLength, ~focus=6, ())
            |> handleInput(~key) == collapsed(testStringLength);
          };
          let%test "Moves cursor to the end and decrease selection" = {
            notCollapsed(~anchor=7, ~focus=3, ())
            |> handleInput(~key)
            == notCollapsed(~anchor=7, ~focus=testStringLength, ());
          };
        });
     let%test_module "When <S-C-LEFT>" =
       (module
        {
          let key = "<S-C-LEFT>";
          let%test "Moves cursor to previous word boundary" = {
            collapsed(10)
            |> handleInput(~key) == notCollapsed(~anchor=10, ~focus=5, ());
          };
          let%test "Moves cursor to beginning" = {
            collapsed(3)
            |> handleInput(~key) == notCollapsed(~anchor=3, ~focus=0, ());
          };
          let%test "Doesn't move cursor position when at beginning" = {
            notCollapsed(~anchor=10, ~focus=0, ())
            |> handleInput(~key) == notCollapsed(~anchor=10, ~focus=0, ());
          };
          let%test "Doesn't move cursor position when at beginning and no selection" = {
            collapsed(0) |> handleInput(~key) == collapsed(0);
          };
          let%test "Doesn't move cursor position for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
          let%test "Moves cursor to the previous word boundary and undo selection" = {
            notCollapsed(~anchor=5, ~focus=16, ())
            |> handleInput(~key) == notCollapsed(~anchor=5, ~focus=5, ());
          };

          let%test "Moves cursor to the previous word boundary and decrease selesction" = {
            notCollapsed(~anchor=11, ~focus=16, ())
            |> handleInput(~key) == notCollapsed(~anchor=11, ~focus=5, ());
          };
        });
     let%test_module "When <C-LEFT>" =
       (module
        {
          let key = "<C-LEFT>";
          let%test "Moves cursor to previous word boundary" = {
            collapsed(10) |> handleInput(~key) == collapsed(5);
          };
          let%test "Moves cursor to beginning" = {
            collapsed(3) |> handleInput(~key) == collapsed(0);
          };
          let%test "Doesn't move cursor position when at beginning" = {
            notCollapsed(~anchor=10, ~focus=0, ())
            |> handleInput(~key) == collapsed(0);
          };
          let%test "Doesn't move cursor position when at beginning and no selection" = {
            collapsed(0) |> handleInput(~key) == collapsed(0);
          };
          let%test "Doesn't move cursor position for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
          let%test "Moves cursor to the previous word boundary and undo selection" = {
            notCollapsed(~anchor=5, ~focus=16, ())
            |> handleInput(~key) == collapsed(5);
          };

          let%test "Moves cursor to the previous word boundary and decrease selesction" = {
            notCollapsed(~anchor=11, ~focus=16, ())
            |> handleInput(~key) == collapsed(5);
          };
        });
     let%test_module "When <S-C-RIGHT>" =
       (module
        {
          let key = "<S-C-RIGHT>";
          let%test "Moves cursor to next word boundary" = {
            collapsed(10)
            |> handleInput(~key) == notCollapsed(~anchor=10, ~focus=16, ());
          };
          let%test "Moves cursor to end" = {
            collapsed(44)
            |> handleInput(~key)
            == notCollapsed(~anchor=44, ~focus=testStringLength, ());
          };
          let%test "Doesn't move cursor position when at the end" = {
            notCollapsed(~anchor=10, ~focus=testStringLength, ())
            |> handleInput(~key)
            == notCollapsed(~anchor=10, ~focus=testStringLength, ());
          };
          let%test "Doesn't move cursor position when at the end and no selection" = {
            collapsed(testStringLength)
            |> handleInput(~key) == collapsed(testStringLength);
          };
          let%test "Doesn't move cursor position for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
          let%test "Moves cursor to the next word boundary and undo selection" = {
            notCollapsed(~anchor=16, ~focus=5, ())
            |> handleInput(~key) == collapsed(16);
          };

          let%test "Moves cursor to the next word boundary and decrease selesction" = {
            notCollapsed(~anchor=10, ~focus=6, ())
            |> handleInput(~key) == notCollapsed(~anchor=10, ~focus=16, ());
          };
        });
     let%test_module "When <C-RIGHT>" =
       (module
        {
          let key = "<C-RIGHT>";
          let%test "Moves cursor to next word boundary" = {
            collapsed(10) |> handleInput(~key) == collapsed(16);
          };
          let%test "Moves cursor to end" = {
            collapsed(44)
            |> handleInput(~key) == collapsed(testStringLength);
          };
          let%test "Doesn't move cursor position when at the end" = {
            notCollapsed(~anchor=10, ~focus=testStringLength, ())
            |> handleInput(~key) == collapsed(testStringLength);
          };
          let%test "Doesn't move cursor position when at the end and no selection" = {
            collapsed(testStringLength)
            |> handleInput(~key) == collapsed(testStringLength);
          };
          let%test "Doesn't move cursor position for blank string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
          let%test "Moves cursor to the next word boundary and undo selection" = {
            notCollapsed(~anchor=16, ~focus=5, ())
            |> handleInput(~key) == collapsed(16);
          };

          let%test "Moves cursor to the next word boundary and decrease selesction" = {
            notCollapsed(~anchor=10, ~focus=6, ())
            |> handleInput(~key) == collapsed(16);
          };
        });
     let%test_module "ASCII letter with no selection" =
       (module
        {
          let key = "F";
          let%test "Adds character to the beginning" = {
            collapsed(0)
            |> handleInput(~key)
            == collapsed(
                 ~text="FSome interesting. Test. String. Isn't it? Maybe",
                 1,
               );
          };
          let%test "Adds character to the end" = {
            collapsed(testStringLength)
            |> handleInput(~key)
            == collapsed(
                 ~text="Some interesting. Test. String. Isn't it? MaybeF",
                 testStringLength + 1,
               );
          };
          let%test "Adds character to cursor position" = {
            collapsed(7)
            |> handleInput(~key)
            == collapsed(
                 ~text="Some inFteresting. Test. String. Isn't it? Maybe",
                 8,
               );
          };
        });
     let%test_module "ASCII letter with  selection" =
       (module
        {
          let key = "F";
          let%test "Adds character to the beginning" = {
            notCollapsed(~anchor=0, ~focus=1, ())
            |> handleInput(~key)
            == collapsed(
                 ~text="Fome interesting. Test. String. Isn't it? Maybe",
                 1,
               );
          };
          let%test "Replaces many characters" = {
            notCollapsed(~anchor=16, ~focus=4, ())
            |> handleInput(~key)
            == collapsed(~text="SomeF. Test. String. Isn't it? Maybe", 5);
          };
          let%test "Replaces all string" = {
            notCollapsed(~anchor=testStringLength, ~focus=0, ())
            |> handleInput(~key) == collapsed(~text="F", 1);
          };
        });
     let%test_module "When <C-a>" =
       (module
        {
          let key = "<C-a>";
          let%test "Select all when no selection" = {
            collapsed(3)
            |> handleInput(~key)
            == notCollapsed(~anchor=0, ~focus=testStringLength, ());
          };
          let%test "Select all when is selection" = {
            notCollapsed(~anchor=5, ~focus=24, ())
            |> handleInput(~key)
            == notCollapsed(~anchor=0, ~focus=testStringLength, ());
          };
          let%test "Selects nothing with empty string" = {
            collapsed(~text="", 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
        });
   });
