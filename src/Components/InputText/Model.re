open Oni_Core;

[@deriving show]
type msg =
  | GainedFocus
  | LostFocus
  | Sneaked
  | Clicked({selection: Selection.t});

type outmsg =
  | Nothing
  | Focus;

[@deriving show]
type t = {
  isFocused: bool,
  value: string,
  selection: Selection.t,
  placeholder: string,
};

let create = (~placeholder) => {
  isFocused: false,
  value: "",
  selection: Selection.initial,
  placeholder,
};

let empty = create(~placeholder="");

let isEmpty = ({value, _}) => value == "";
let isFocused = ({isFocused, _}) => isFocused;

let value = ({value, _}) => value;

let update = (msg, model) =>
  switch (msg) {
  | GainedFocus => ({...model, isFocused: true}, Nothing)
  | LostFocus => ({...model, isFocused: false}, Nothing)
  | Sneaked => (model, Focus)
  | Clicked({selection}) => ({...model, selection}, Focus)
  };

module Internal = {
  let wordSeparators = " ./\\()\"'-:,.;<>~!@#$%^&*|+=[]{}`~?";

  let separatorOnIndexExn = (index, text) => {
    Zed_utf8.contains(wordSeparators, Zed_utf8.sub(text, index, 1));
  };

  let findNextWordBoundary = (text, focus) => {
    let finalIndex = Zed_utf8.length(text);
    let index = ref(min(focus + 1, finalIndex));

    while (index^ < finalIndex && !separatorOnIndexExn(index^, text)) {
      index := index^ + 1;
    };

    index^;
  };

  let findPrevWordBoundary = (text, focus) => {
    let finalIndex = 0;
    let index = ref(max(focus - 1, finalIndex));

    while (index^ > finalIndex && separatorOnIndexExn(index^, text)) {
      index := index^ - 1;
    };
    while (index^ > finalIndex && !separatorOnIndexExn(index^ - 1, text)) {
      index := index^ - 1;
    };

    index^;
  };

  let removeBefore = (~count=1, index, text) => {
    let safeCount = min(0, index - count) + count;

    (Zed_utf8.remove(text, index - safeCount, safeCount), index - safeCount);
  };

  let removeAfter = (~count=1, index, text) => {
    let safeCount = min(0, Zed_utf8.length(text) - (index + count)) + count;

    (Zed_utf8.remove(text, index, safeCount), index);
  };

  let add = (~at as index, insert, text) => (
    Zed_utf8.insert(text, index, insert),
    index + Zed_utf8.length(insert),
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

  let removeLineBefore = (text, selection: Selection.t) => {
    let index = selection.focus;
    let count = index;

    let (textSlice, idx) = removeBefore(~count, index, text);

    (textSlice, Selection.collapsed(~text=textSlice, idx));
  };

  let removeLineAfter = (text, selection: Selection.t) => {
    let index = selection.focus;
    let count = Zed_utf8.length(text) - index;

    let (textSlice, idx) = removeAfter(~count, index, text);

    (textSlice, Selection.collapsed(~text=textSlice, idx));
  };

  let removeLine = text => {
    let index = 0;
    let count = Zed_utf8.length(text);

    let (textSlice, idx) = removeAfter(~count, index, text);

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
    // Arrow keys
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

    // Deletions
    | ("<BS>", true) => removeCharBefore(text, selection)
    | ("<BS>", false) => removeSelection(text, selection)
    | ("<C-BS>", true) => removeWord(text, selection)
    | ("<C-BS>", false) => removeSelection(text, selection)
    | ("<A-BS>", true) => removeWord(text, selection)
    | ("<A-BS>", false) => removeSelection(text, selection)
    | ("<D-BS>", false) => removeSelection(text, selection)
    | ("<D-BS>", true) =>
      removeSelection(
        text,
        Selection.create(~text, ~anchor=0, ~focus=selection.focus),
      )
    | ("<C-h>", true) => removeCharBefore(text, selection)
    | ("<C-h>", false) => removeSelection(text, selection)
    | ("<C-w>", true) => removeWord(text, selection)
    | ("<C-w>", false) => removeSelection(text, selection)
    | ("<C-u>", true) => removeLineBefore(text, selection)
    | ("<C-u>", false) => removeSelection(text, selection)
    | ("<C-k>", true) => removeLineAfter(text, selection)
    | ("<C-k>", false) => removeSelection(text, selection)
    | ("<C-c>", _) => removeLine(text)
    | ("<DEL>", true) => removeCharAfter(text, selection)
    | ("<DEL>", false) => removeSelection(text, selection)

    // Move focus to start/end of line
    | ("<HOME>", _) => (text, Selection.collapsed(~text, 0))
    | ("<END>", _) => (
        text,
        Selection.collapsed(~text, Zed_utf8.length(text)),
      )
    | ("<D-LEFT>", _) => (text, Selection.collapsed(~text, 0))
    | ("<D-RIGHT>", _) => (
        text,
        Selection.collapsed(~text, Zed_utf8.length(text)),
      )

    // Extend selection one character to left/right
    | ("<S-LEFT>", _) => (
        text,
        selection.focus - 1 |> Selection.extend(~text, ~selection),
      )
    | ("<S-RIGHT>", _) => (
        text,
        selection.focus + 1 |> Selection.extend(~text, ~selection),
      )

    // Move focus one word to left/right
    | ("<C-LEFT>", _) => collapsePrevWord(text, selection)
    | ("<C-RIGHT>", _) => collapseNextWord(text, selection)
    | ("<A-LEFT>", _) => collapsePrevWord(text, selection)
    | ("<A-RIGHT>", _) => collapseNextWord(text, selection)

    // Extend selection to start/end of line
    | ("<S-HOME>", _) => (text, Selection.extend(~text, ~selection, 0))
    | ("<S-END>", _) => (
        text,
        Selection.extend(~text, ~selection, Zed_utf8.length(text)),
      )
    | ("<D-S-LEFT>", _) => (text, Selection.extend(~text, ~selection, 0))
    | ("<D-S-RIGHT>", _) => (
        text,
        Selection.extend(~text, ~selection, Zed_utf8.length(text)),
      )

    // Extend selection one word to left/right
    | ("<S-C-LEFT>", _) => extendPrevWord(text, selection)
    | ("<S-C-RIGHT>", _) => extendNextWord(text, selection)
    | ("<A-S-LEFT>", _) => extendPrevWord(text, selection)
    | ("<A-S-RIGHT>", _) => extendNextWord(text, selection)

    // Select all
    | ("<C-a>", _) => (
        text,
        Selection.create(~text, ~anchor=0, ~focus=Zed_utf8.length(text)),
      )
    | ("<D-a>", _) => (
        text,
        Selection.create(~text, ~anchor=0, ~focus=Zed_utf8.length(text)),
      )

    // Insert character / replace selection with character
    | (key, true) when Zed_utf8.length(key) == 1 =>
      addCharacter(key, text, selection)
    | (key, false) when Zed_utf8.length(key) == 1 =>
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

let set = (~cursor=?, ~text, model) => {
  let idx =
    switch (cursor) {
    | Some(idx) => idx
    | None => Zed_utf8.length(text)
    };
  let selection = Selection.collapsed(~text, idx);
  {...model, value: text, selection};
};

let setPlaceholder = (~placeholder, model) => {...model, placeholder};

let isCursorAtEnd = ({value, selection, _}) => {
  Selection.isCollapsed(selection)
  && selection.focus == Zed_utf8.length(value);
};

let cursorPosition = ({selection, _}) => selection.focus;

// TESTS

let%test_module "Model" =
  (module
   {
     let testString = "Some interesting. Test. String. Isn't it? Maybe";
     let testStringLength = Zed_utf8.length(testString);

     let collapsed = (~text=testString, position) => {
       value: text,
       selection: Selection.create(~text, ~anchor=position, ~focus=position),
       placeholder: "",
       isFocused: false,
     };

     let notCollapsed = (~text=testString, ~anchor, ~focus, ()) => {
       value: text,
       selection: Selection.create(~text, ~anchor, ~focus),
       placeholder: "",
       isFocused: false,
     };

     let uTestString = "😊↪Вім is Cool";

     let%test_module "paste" =
       (module
        {
          let pasteText = "😊↪Вім hello from clipboard";
          let pasteTextLength = pasteText |> Zed_utf8.length;
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
          let%test "Removes multiple spaces" = {
            collapsed(~text="testing   three spaces", 10)
            |> handleInput(~key) == collapsed(~text="three spaces", 0);
          };
          let%test "Removes word with unicode characters on the left of cursor" = {
            collapsed(~text=uTestString, 6)
            |> handleInput(~key) == collapsed(~text="is Cool", 0);
          };

          let%test "Doesn't remove anything if cursor at the beginning" = {
            collapsed(0) |> handleInput(~key) == collapsed(0);
          };

          let%test "Don't do anything for blank string" = {
            collapsed(~text="", -1)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
        });
     let%test_module "When <C-u> with no selection" =
       (module
        {
          let key = "<C-u>";
          let%test "Removes all characters on the left of cursor" = {
            collapsed(10)
            |> handleInput(~key)
            == collapsed(~text="esting. Test. String. Isn't it? Maybe", 0);
          };
          let%test "Removes all characters including Unicode on the left of cursor" = {
            collapsed(~text=uTestString, 9)
            |> handleInput(~key) == collapsed(~text="Cool", 0);
          };

          let%test "Doesn't remove anything if cursor at the beginning" = {
            collapsed(0) |> handleInput(~key) == collapsed(0);
          };

          let%test "Don't do anything for blank string" = {
            collapsed(~text="", -1)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
        });
     let%test_module "When <C-k> with no selection" =
       (module
        {
          let key = "<C-k>";
          let%test "Removes all characters on the right of cursor" = {
            collapsed(10)
            |> handleInput(~key) == collapsed(~text="Some inter", 10);
          };
          let%test "Removes all characters including Unicode on the right of cursor" = {
            collapsed(~text=uTestString, 0)
            |> handleInput(~key) == collapsed(~text="", 0);
          };

          let%test "Doesn't remove anything if cursor at the end" = {
            collapsed(testStringLength)
            |> handleInput(~key) == collapsed(testStringLength);
          };

          let%test "Don't do anything for blank string" = {
            collapsed(~text="", -1)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
        });
     let%test_module "When <C-c>" =
       (module
        {
          let key = "<C-c>";
          let%test "Removes all characters" = {
            collapsed(10) |> handleInput(~key) == collapsed(~text="", 0);
          };

          let%test "Removes all characters including Unicode" = {
            collapsed(~text=uTestString, 2)
            |> handleInput(~key) == collapsed(~text="", 0);
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
          let%test "Removes emoji character on the left of cursor" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text=uTestString, 1)
                 |> handleInput(~key)
                 == collapsed(~text="↪Вім is Cool", 0)
               });
          };
          let%test "Removes cyrilic character on the left of cursor" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text=uTestString, 3)
                 |> handleInput(~key)
                 == collapsed(~text="😊↪ім is Cool", 2)
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
          let%test "Removes selection for unicode characters" = {
            notCollapsed(~text=uTestString, ~anchor=0, ~focus=7, ())
            |> handleInput(~key)
            == notCollapsed(~anchor=0, ~focus=0, ~text="s Cool", ());
          };
        });
     let%test_module "When <C-BS> or <A-BS> with no selection" =
       (module
        {
          let keys = ["<C-BS>", "<A-BS>"];
          let%test "Removes word on the left of cursor" = {
            keys
            |> List.for_all(key => {
                 collapsed(16)
                 |> handleInput(~key)
                 == collapsed(~text="Some . Test. String. Isn't it? Maybe", 5)
               });
          };
          let%test "Doesn't remove word if cursor at the beginning" = {
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
          let%test "Removes word with cyrilic character in it" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text=uTestString, 5)
                 |> handleInput(~key) == collapsed(~text=" is Cool", 0)
               });
          };
          let%test "Removes word made of emojis" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text=uTestString, 2)
                 |> handleInput(~key) == collapsed(~text="Вім is Cool", 0)
               });
          };
        });
     let%test_module "When <C-BS> or <A-BS> with selection" =
       (module
        {
          let keys = ["<C-BS>", "<A-BS>"];
          let%test "Removes selection when cursor comes first" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=4, ~focus=2, ())
                 |> handleInput(~key)
                 == notCollapsed(
                      ~anchor=2,
                      ~focus=2,
                      ~text="So interesting. Test. String. Isn't it? Maybe",
                      (),
                    )
               });
          };
          let%test "Removes selection when cursor comes last" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=2, ~focus=4, ())
                 |> handleInput(~key)
                 == notCollapsed(
                      ~anchor=2,
                      ~focus=2,
                      ~text="So interesting. Test. String. Isn't it? Maybe",
                      (),
                    )
               });
          };
          let%test "Removes selection when more than one word selected" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=5, ~focus=23, ())
                 |> handleInput(~key)
                 == notCollapsed(
                      ~anchor=5,
                      ~focus=5,
                      ~text="Some  String. Isn't it? Maybe",
                      (),
                    )
               });
          };
          let%test "Removes selection when unicode character" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~text=uTestString, ~anchor=0, ~focus=7, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=0, ~focus=0, ~text="s Cool", ())
               });
          };
        });
     let%test_module "When <D-BS> with no selection" =
       (module
        {
          let key = "<D-BS>";
          let%test "Removes all text when focus on last char" = {
            collapsed(testStringLength)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
          let%test "Removes all text to the left from focus" = {
            collapsed(20)
            |> handleInput(~key)
            == collapsed(~text="st. String. Isn't it? Maybe", 0);
          };
          let%test "Removes text with cyrilic and emojis" = {
            collapsed(~text=uTestString, 14)
            |> handleInput(~key) == collapsed(~text="", 0);
          };
        });
     let%test_module "When <D-BS> with selection" =
       (module
        {
          let key = "<D-BS>";
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
          let%test "Removes selection when more than one word selected" = {
            notCollapsed(~anchor=5, ~focus=23, ())
            |> handleInput(~key)
            == notCollapsed(
                 ~anchor=5,
                 ~focus=5,
                 ~text="Some  String. Isn't it? Maybe",
                 (),
               );
          };
          let%test "Removes selection when unicode character" = {
            notCollapsed(~text=uTestString, ~anchor=0, ~focus=7, ())
            |> handleInput(~key)
            == notCollapsed(~anchor=0, ~focus=0, ~text="s Cool", ());
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
          let%test "Removes emoji character on the right side of cursor" = {
            collapsed(~text=uTestString, 0)
            |> handleInput(~key) == collapsed(~text="↪Вім is Cool", 0);
          };
          let%test "Removes cyrilic character on the right side of cursor" = {
            collapsed(~text=uTestString, 2)
            |> handleInput(~key) == collapsed(~text="😊↪ім is Cool", 2);
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
          let%test "Removes selection when unicode character" = {
            notCollapsed(~text=uTestString, ~anchor=0, ~focus=7, ())
            |> handleInput(~key)
            == notCollapsed(~anchor=0, ~focus=0, ~text="s Cool", ());
          };
        });
     let%test_module "When <HOME> or <D-LEFT> with no selection" =
       (module
        {
          let keys = ["<HOME>", "<D-LEFT>"];
          let%test "Moves cursor to the beginning" = {
            keys
            |> List.for_all(key => {
                 collapsed(4) |> handleInput(~key) == collapsed(0)
               });
          };
          let%test "Doesn't move cursor if it's at the beginning" = {
            keys
            |> List.for_all(key => {
                 collapsed(0) |> handleInput(~key) == collapsed(0)
               });
          };
          let%test "Doesn't move cursor position for blank string" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text="", 0)
                 |> handleInput(~key) == collapsed(~text="", 0)
               });
          };
        });
     let%test_module "When <HOME> or <D-LEFT> with selection" =
       (module
        {
          let keys = ["<HOME>", "<D-LEFT>"];
          let%test "Moves cursor to the beginning and discards selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=12, ~focus=5, ())
                 |> handleInput(~key) == collapsed(0)
               });
          };
          let%test "Doesn't move cursor if it's at the beginning and discard selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=8, ~focus=0, ())
                 |> handleInput(~key) == collapsed(0)
               });
          };
        });
     let%test_module "When <END> or <D-RIGHT> with no selection" =
       (module
        {
          let keys = ["<END>", "<D-RIGHT>"];
          let%test "Moves cursor to the end" = {
            keys
            |> List.for_all(key => {
                 collapsed(4)
                 |> handleInput(~key) == collapsed(testStringLength)
               });
          };
          let%test "Doesn't move cursor if its at the end" = {
            keys
            |> List.for_all(key => {
                 collapsed(testStringLength)
                 |> handleInput(~key) == collapsed(testStringLength)
               });
          };
          let%test "Doesn't move cursor for blank string" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text="", testStringLength)
                 |> handleInput(~key)
                 == collapsed(~text="", testStringLength)
               });
          };
        });
     let%test_module "When <END> or <D-RIGHT> with selection" =
       (module
        {
          let keys = ["<END>", "<D-RIGHT>"];
          let%test "Moves cursor to the end" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=11, ~focus=5, ())
                 |> handleInput(~key)
                 == notCollapsed(
                      ~anchor=testStringLength,
                      ~focus=testStringLength,
                      (),
                    )
               });
          };
          let%test "Doesn't move cursor if it's at the end and discard selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=testStringLength, ~focus=4, ())
                 |> handleInput(~key)
                 == notCollapsed(
                      ~anchor=testStringLength,
                      ~focus=testStringLength,
                      (),
                    )
               });
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
     let%test_module "When <S-HOME> or <D-S-LEFT>" =
       (module
        {
          let keys = ["<S-HOME>", "<D-S-LEFT>"];
          let%test "Moves cursor to the beginning and add selection" = {
            keys
            |> List.for_all(key => {
                 collapsed(4)
                 |> handleInput(~key)
                 == notCollapsed(~anchor=4, ~focus=0, ())
               });
          };
          let%test "Moves cursor to the beginning and increase selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=7, ~focus=4, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=7, ~focus=0, ())
               });
          };
          let%test "Doesn't move cursor position when at the beginning" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=5, ~focus=0, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=5, ~focus=0, ())
               });
          };
          let%test "Doesn't move cursor position when at beginning and no selection" = {
            keys
            |> List.for_all(key => {
                 collapsed(0) |> handleInput(~key) == collapsed(0)
               });
          };
          let%test "Doesn't move cursor position for blank string" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text="", 0)
                 |> handleInput(~key) == collapsed(~text="", 0)
               });
          };
          let%test "Moves cursor to the beginning and undo selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=0, ~focus=6, ())
                 |> handleInput(~key) == collapsed(0)
               });
          };
          let%test "Moves cursor to the beginning and decrease selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=3, ~focus=8, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=3, ~focus=0, ())
               });
          };
        });
     let%test_module "When <S-END> or <D-S-RIGHT>" =
       (module
        {
          let keys = ["<S-END>", "<D-S-RIGHT>"];
          let%test "Moves cursor to the end and add selection" = {
            keys
            |> List.for_all(key => {
                 collapsed(4)
                 |> handleInput(~key)
                 == notCollapsed(~anchor=4, ~focus=testStringLength, ())
               });
          };
          let%test "Moves cursor to the end and increase selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=4, ~focus=8, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=4, ~focus=testStringLength, ())
               });
          };
          let%test "Doesn't move cursor position when it is at the end" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=5, ~focus=testStringLength, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=5, ~focus=testStringLength, ())
               });
          };
          let%test "Doesn't move cursor position when it is at the end and no selection" = {
            keys
            |> List.for_all(key => {
                 collapsed(testStringLength)
                 |> handleInput(~key) == collapsed(testStringLength)
               });
          };
          let%test "Doesn't move cursor position for blank string" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text="", 0)
                 |> handleInput(~key) == collapsed(~text="", 0)
               });
          };
          let%test "Moves cursor to the end and undo selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=testStringLength, ~focus=6, ())
                 |> handleInput(~key) == collapsed(testStringLength)
               });
          };
          let%test "Moves cursor to the end and decrease selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=7, ~focus=3, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=7, ~focus=testStringLength, ())
               });
          };
        });
     let%test_module "When <S-C-LEFT> or <A-S-LEFT>" =
       (module
        {
          let keys = ["<S-C-LEFT>", "<A-S-LEFT>"];
          let%test "Moves cursor to previous word boundary" = {
            keys
            |> List.for_all(key => {
                 collapsed(10)
                 |> handleInput(~key)
                 == notCollapsed(~anchor=10, ~focus=5, ())
               });
          };
          let%test "Moves cursor to beginning" = {
            keys
            |> List.for_all(key => {
                 collapsed(3)
                 |> handleInput(~key)
                 == notCollapsed(~anchor=3, ~focus=0, ())
               });
          };
          let%test "Doesn't move cursor position when at beginning" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=10, ~focus=0, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=10, ~focus=0, ())
               });
          };
          let%test "Doesn't move cursor position when at beginning and no selection" = {
            keys
            |> List.for_all(key => {
                 collapsed(0) |> handleInput(~key) == collapsed(0)
               });
          };
          let%test "Doesn't move cursor position for blank string" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text="", 0)
                 |> handleInput(~key) == collapsed(~text="", 0)
               });
          };
          let%test "Moves cursor to the previous word boundary and undo selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=5, ~focus=16, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=5, ~focus=5, ())
               });
          };

          let%test "Moves cursor to the previous word boundary and decrease selesction" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=11, ~focus=16, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=11, ~focus=5, ())
               });
          };
        });
     let%test_module "When <C-LEFT> or <A-LEFT>" =
       (module
        {
          let keys = ["<C-LEFT>", "<A-LEFT>"];
          let%test "Moves cursor to previous word boundary" = {
            keys
            |> List.for_all(key => {
                 collapsed(10) |> handleInput(~key) == collapsed(5)
               });
          };
          let%test "Moves cursor to beginning" = {
            keys
            |> List.for_all(key => {
                 collapsed(3) |> handleInput(~key) == collapsed(0)
               });
          };
          let%test "Doesn't move cursor position when at beginning" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=10, ~focus=0, ())
                 |> handleInput(~key) == collapsed(0)
               });
          };
          let%test "Doesn't move cursor position when at beginning and no selection" = {
            keys
            |> List.for_all(key => {
                 collapsed(0) |> handleInput(~key) == collapsed(0)
               });
          };
          let%test "Doesn't move cursor position for blank string" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text="", 0)
                 |> handleInput(~key) == collapsed(~text="", 0)
               });
          };
          let%test "Moves cursor to the previous word boundary and undo selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=5, ~focus=16, ())
                 |> handleInput(~key) == collapsed(5)
               });
          };

          let%test "Moves cursor to the previous word boundary and decrease selesction" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=11, ~focus=16, ())
                 |> handleInput(~key) == collapsed(5)
               });
          };
        });
     let%test_module "When <S-C-RIGHT> or <A-S-RIGHT>" =
       (module
        {
          let keys = ["<S-C-RIGHT>", "<A-S-RIGHT>"];
          let%test "Moves cursor to next word boundary" = {
            keys
            |> List.for_all(key => {
                 collapsed(10)
                 |> handleInput(~key)
                 == notCollapsed(~anchor=10, ~focus=16, ())
               });
          };
          let%test "Moves cursor to end" = {
            keys
            |> List.for_all(key => {
                 collapsed(44)
                 |> handleInput(~key)
                 == notCollapsed(~anchor=44, ~focus=testStringLength, ())
               });
          };
          let%test "Doesn't move cursor position when at the end" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=10, ~focus=testStringLength, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=10, ~focus=testStringLength, ())
               });
          };
          let%test "Doesn't move cursor position when at the end and no selection" = {
            keys
            |> List.for_all(key => {
                 collapsed(testStringLength)
                 |> handleInput(~key) == collapsed(testStringLength)
               });
          };
          let%test "Doesn't move cursor position for blank string" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text="", 0)
                 |> handleInput(~key) == collapsed(~text="", 0)
               });
          };
          let%test "Moves cursor to the next word boundary and undo selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=16, ~focus=5, ())
                 |> handleInput(~key) == collapsed(16)
               });
          };

          let%test "Moves cursor to the next word boundary and decrease selesction" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=10, ~focus=6, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=10, ~focus=16, ())
               });
          };
        });
     let%test_module "When <C-RIGHT> or <A-RIGHT>" =
       (module
        {
          let keys = ["<C-RIGHT>", "<A-RIGHT>"];
          let%test "Moves cursor to next word boundary" = {
            keys
            |> List.for_all(key => {
                 collapsed(10) |> handleInput(~key) == collapsed(16)
               });
          };
          let%test "Moves cursor to end" = {
            keys
            |> List.for_all(key => {
                 collapsed(44)
                 |> handleInput(~key) == collapsed(testStringLength)
               });
          };
          let%test "Doesn't move cursor position when at the end" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=10, ~focus=testStringLength, ())
                 |> handleInput(~key) == collapsed(testStringLength)
               });
          };
          let%test "Doesn't move cursor position when at the end and no selection" = {
            keys
            |> List.for_all(key => {
                 collapsed(testStringLength)
                 |> handleInput(~key) == collapsed(testStringLength)
               });
          };
          let%test "Doesn't move cursor position for blank string" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text="", 0)
                 |> handleInput(~key) == collapsed(~text="", 0)
               });
          };
          let%test "Moves cursor to the next word boundary and undo selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=16, ~focus=5, ())
                 |> handleInput(~key) == collapsed(16)
               });
          };

          let%test "Moves cursor to the next word boundary and decrease selesction" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=10, ~focus=6, ())
                 |> handleInput(~key) == collapsed(16)
               });
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
     let%test_module "Emoji letter with no selection" =
       (module
        {
          let key = "😊";
          let%test "Adds character to the beginning" = {
            collapsed(0)
            |> handleInput(~key)
            == collapsed(
                 ~text="😊Some interesting. Test. String. Isn't it? Maybe",
                 1,
               );
          };
          let%test "Adds character to the end" = {
            collapsed(testStringLength)
            |> handleInput(~key)
            == collapsed(
                 ~text="Some interesting. Test. String. Isn't it? Maybe😊",
                 testStringLength + 1,
               );
          };
          let%test "Adds character to cursor position" = {
            collapsed(7)
            |> handleInput(~key)
            == collapsed(
                 ~text="Some in😊teresting. Test. String. Isn't it? Maybe",
                 8,
               );
          };
        });
     let%test_module "Cyrilic letter with no selection" =
       (module
        {
          let key = "Ї";
          let%test "Adds character to the beginning" = {
            collapsed(0)
            |> handleInput(~key)
            == collapsed(
                 ~text="ЇSome interesting. Test. String. Isn't it? Maybe",
                 1,
               );
          };
          let%test "Adds character to the end" = {
            collapsed(testStringLength)
            |> handleInput(~key)
            == collapsed(
                 ~text="Some interesting. Test. String. Isn't it? MaybeЇ",
                 testStringLength + 1,
               );
          };
          let%test "Adds character to cursor position" = {
            collapsed(7)
            |> handleInput(~key)
            == collapsed(
                 ~text="Some inЇteresting. Test. String. Isn't it? Maybe",
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
     let%test_module "Emoji letter with  selection" =
       (module
        {
          let key = "😊";
          let%test "Adds character to the beginning" = {
            notCollapsed(~anchor=0, ~focus=1, ())
            |> handleInput(~key)
            == collapsed(
                 ~text="😊ome interesting. Test. String. Isn't it? Maybe",
                 1,
               );
          };
          let%test "Replaces many characters" = {
            notCollapsed(~anchor=16, ~focus=4, ())
            |> handleInput(~key)
            == collapsed(~text="Some😊. Test. String. Isn't it? Maybe", 5);
          };
          let%test "Replaces all string" = {
            notCollapsed(~anchor=testStringLength, ~focus=0, ())
            |> handleInput(~key) == collapsed(~text="😊", 1);
          };
        });
     let%test_module "Cyrilic letter with  selection" =
       (module
        {
          let key = "Ї";
          let%test "Adds character to the beginning" = {
            notCollapsed(~anchor=0, ~focus=1, ())
            |> handleInput(~key)
            == collapsed(
                 ~text="Їome interesting. Test. String. Isn't it? Maybe",
                 1,
               );
          };
          let%test "Replaces many characters" = {
            notCollapsed(~anchor=16, ~focus=4, ())
            |> handleInput(~key)
            == collapsed(~text="SomeЇ. Test. String. Isn't it? Maybe", 5);
          };
          let%test "Replaces all string" = {
            notCollapsed(~anchor=testStringLength, ~focus=0, ())
            |> handleInput(~key) == collapsed(~text="Ї", 1);
          };
        });
     let%test_module "When <C-a> or <D-a>" =
       (module
        {
          let keys = ["<C-a>", "<D-a>"];
          let%test "Select all when no selection" = {
            keys
            |> List.for_all(key => {
                 collapsed(3)
                 |> handleInput(~key)
                 == notCollapsed(~anchor=0, ~focus=testStringLength, ())
               });
          };
          let%test "Select all when is selection" = {
            keys
            |> List.for_all(key => {
                 notCollapsed(~anchor=5, ~focus=24, ())
                 |> handleInput(~key)
                 == notCollapsed(~anchor=0, ~focus=testStringLength, ())
               });
          };
          let%test "Selects nothing with empty string" = {
            keys
            |> List.for_all(key => {
                 collapsed(~text="", 0)
                 |> handleInput(~key) == collapsed(~text="", 0)
               });
          };
        });
   });
