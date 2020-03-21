open TestFramework;

module InputModel = Oni_Components.InputModel;
module Selection = Oni_Components.Selection;

let testString = "Some interesting. Test. String. Isn't it? Maybe";
let testStringLength = String.length(testString);

let runInputHandler = (~text=testString, selection, key) => {
  InputModel.handleInput(~text, ~selection, key);
};

let collapsedSelection = (~text=testString, position) => {
  Selection.create(~text, ~anchor=position, ~focus=position);
};

let notCollapsedSelection = (~text=testString, ~anchor, ~focus, ()) => {
  Selection.create(~text, ~anchor, ~focus);
};

describe("handleInputS#handleInput", ({describe, _}) => {
  describe("When LEFT with no selection", ({test, _}) => {
    let key = "<LEFT>";

    test("Moves cursor left for 1 character", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected = collapsedSelection(3);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor less then 0 position", ({expect, _}) => {
      let selection = collapsedSelection(0);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });
  });

  describe("When LEFT with selection", ({test, _}) => {
    let key = "<LEFT>";

    test(
      "Moves cursor to the beginning of selection when cursor comes first",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=4, ~focus=2, ());
      let expected = notCollapsedSelection(~anchor=2, ~focus=2, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to the beginning of selection when cursor comes last",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=2, ~focus=4, ());
      let expected = notCollapsedSelection(~anchor=2, ~focus=2, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to the beginning of selection when cursor at the beginning",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=2, ~focus=4, ());
      let expected = notCollapsedSelection(~anchor=2, ~focus=2, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });
  });

  describe("When <RIGHT> with no selection", ({test, _}) => {
    let key = "<RIGHT>";

    test("Moves cursor right for 1 character", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected = collapsedSelection(5);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position more that string length", ({expect, _}) => {
      let selection = collapsedSelection(testStringLength);
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });
  });

  describe("When RIGHT with selection", ({test, _}) => {
    let key = "<RIGHT>";

    test(
      "Moves cursor to the end of selection when cursor comes first",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=2, ~focus=4, ());
      let expected = notCollapsedSelection(~anchor=4, ~focus=4, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to the end of selection when cursor comes last",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=4, ~focus=2, ());
      let expected = notCollapsedSelection(~anchor=4, ~focus=4, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to the end of selection when cursor at the beginning",
      ({expect, _}) => {
      let selection =
        notCollapsedSelection(~anchor=testStringLength, ~focus=0, ());
      let expected =
        notCollapsedSelection(
          ~anchor=testStringLength,
          ~focus=testStringLength,
          (),
        );

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });
  });

  describe("When <RIGHT> with no selection", ({test, _}) => {
    let key = "<RIGHT>";

    test("Moves cursor right for 1 character", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected = collapsedSelection(5);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position more that string length", ({expect, _}) => {
      let selection = collapsedSelection(testStringLength);
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });
  });
  describe("When <C-w> with no selection", ({test, _}) => {
    let key = "<C-w>";
    test("Removes word characters on the left of cursor", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(
        " interesting. Test. String. Isn't it? Maybe",
      );
    });

    test("Doesn't remove anything if cursor at the beginning", ({expect, _}) => {
      let selection = collapsedSelection(0);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Don't do anything for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", -1);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });
  });

  describe("When <BS> with no selection", ({test, _}) => {
    let keys = ["<BS>", "<C-h>"];

    keys
    |> List.iter(key => {
         test("Removes character on the left of cursor", ({expect, _}) => {
           let selection = collapsedSelection(4);
           let expected = collapsedSelection(3);

           let (text, newSelection) = runInputHandler(selection, key);

           expect.equal(expected, newSelection);
           expect.string(text).toEqual(
             "Som interesting. Test. String. Isn't it? Maybe",
           );
         });

         test(
           "Doesn't remove character if cursor at the beginnig",
           ({expect, _}) => {
           let selection = collapsedSelection(0);
           let expected = collapsedSelection(0);

           let (text, newSelection) = runInputHandler(selection, key);

           expect.equal(expected, newSelection);
           expect.string(text).toEqual(testString);
         });

         test("Don't do anything for blank string", ({expect, _}) => {
           let selection = collapsedSelection(~text="", -1);
           let expected = collapsedSelection(~text="", 0);

           let (text, newSelection) =
             runInputHandler(~text="", selection, key);

           expect.equal(expected, newSelection);
           expect.string(text).toEqual("");
         });
       });
  });

  describe("When <BS> with with selection", ({test, _}) => {
    let key = "<BS>";

    test("Removes selection when cursor comes first", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=4, ~focus=2, ());
      let expected = notCollapsedSelection(~anchor=2, ~focus=2, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(
        "So interesting. Test. String. Isn't it? Maybe",
      );
    });

    test("Removes selection when cursor comes last", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=2, ~focus=4, ());
      let expected = notCollapsedSelection(~anchor=2, ~focus=2, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(
        "So interesting. Test. String. Isn't it? Maybe",
      );
    });
  });

  describe("When <DEL> with no selection", ({test, _}) => {
    let key = "<DEL>";

    test("Removes character on the right of cursor", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected = collapsedSelection(4);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(
        "Someinteresting. Test. String. Isn't it? Maybe",
      );
    });

    test("Doesn't remove character if cursor at the end", ({expect, _}) => {
      let selection = collapsedSelection(testStringLength);
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Don't do anything for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });
  });

  describe("When <DEL> with with selection", ({test, _}) => {
    let key = "<DEL>";

    test("Removes selection when cursor comes first", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=4, ~focus=2, ());
      let expected = notCollapsedSelection(~anchor=2, ~focus=2, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(
        "So interesting. Test. String. Isn't it? Maybe",
      );
    });

    test("Removes selection when cursor comes last", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=2, ~focus=4, ());
      let expected = notCollapsedSelection(~anchor=2, ~focus=2, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(
        "So interesting. Test. String. Isn't it? Maybe",
      );
    });
  });

  describe("When HOME with no selection", ({test, _}) => {
    let key = "<HOME>";

    test("Moves cursor to the beginning", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor if it's at the beginning", ({expect, _}) => {
      let selection = collapsedSelection(0);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });
  });

  describe("When HOME with selection", ({test, _}) => {
    let key = "<HOME>";

    test(
      "Moves cursor to the beginning and discard selection", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=12, ~focus=5, ());
      let expected = notCollapsedSelection(~anchor=0, ~focus=0, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor if it's at the beginning and discard selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=8, ~focus=0, ());
      let expected = notCollapsedSelection(~anchor=0, ~focus=0, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When END with no selection", ({test, _}) => {
    let key = "<END>";

    test("Moves cursor to the end", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor if it's at the end", ({expect, _}) => {
      let selection = collapsedSelection(testStringLength);
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });
  });

  describe("When END with selection", ({test, _}) => {
    let key = "<END>";

    test("Moves cursor to the end and discard selection", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=11, ~focus=5, ());
      let expected =
        notCollapsedSelection(
          ~anchor=testStringLength,
          ~focus=testStringLength,
          (),
        );

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor if it's at the end and discard selection",
      ({expect, _}) => {
      let selection =
        notCollapsedSelection(~anchor=testStringLength, ~focus=4, ());
      let expected =
        notCollapsedSelection(
          ~anchor=testStringLength,
          ~focus=testStringLength,
          (),
        );

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When S-LEFT", ({test, _}) => {
    let key = "<S-LEFT>";

    test("Moves cursor to 1 character left and add selection", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected = notCollapsedSelection(~anchor=4, ~focus=3, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to 1 character left and increase selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=11, ~focus=5, ());
      let expected = notCollapsedSelection(~anchor=11, ~focus=4, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the beginning", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=5, ~focus=0, ());
      let expected = notCollapsedSelection(~anchor=5, ~focus=0, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the beginning and no selection",
      ({expect, _}) => {
      let selection = collapsedSelection(0);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });

    test(
      "Moves cursor to 1 character left and undo selection", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=4, ~focus=5, ());
      let expected = collapsedSelection(4);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to 1 character left and decrease selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=4, ~focus=8, ());
      let expected = notCollapsedSelection(~anchor=4, ~focus=7, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When S-RIGHT", ({test, _}) => {
    let key = "<S-RIGHT>";

    test(
      "Moves cursor to 1 character right and add selection", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected = notCollapsedSelection(~anchor=4, ~focus=5, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to 1 character right and increase selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=4, ~focus=11, ());
      let expected = notCollapsedSelection(~anchor=4, ~focus=12, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the end", ({expect, _}) => {
      let selection =
        notCollapsedSelection(~anchor=5, ~focus=testStringLength, ());
      let expected =
        notCollapsedSelection(~anchor=5, ~focus=testStringLength, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the end and no selection",
      ({expect, _}) => {
      let selection = collapsedSelection(testStringLength);
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });

    test(
      "Moves cursor to 1 character right and undo selection", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=6, ~focus=5, ());
      let expected = collapsedSelection(6);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to 1 character right and decrease selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=8, ~focus=3, ());
      let expected = notCollapsedSelection(~anchor=8, ~focus=4, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When S-HOME", ({test, _}) => {
    let key = "<S-HOME>";

    test("Moves cursor to the beginning and add selection", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected = notCollapsedSelection(~anchor=4, ~focus=0, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to the beginning and increase selection", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=7, ~focus=4, ());
      let expected = notCollapsedSelection(~anchor=7, ~focus=0, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the beginning", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=5, ~focus=0, ());
      let expected = notCollapsedSelection(~anchor=5, ~focus=0, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the beginning and no selection",
      ({expect, _}) => {
      let selection = collapsedSelection(0);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });

    test("Moves cursor to the beginning and undo selection", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=0, ~focus=6, ());
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to the beginning and decrease selection", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=3, ~focus=8, ());
      let expected = notCollapsedSelection(~anchor=3, ~focus=0, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When S-END", ({test, _}) => {
    let key = "<S-END>";

    test("Moves cursor to the end and add selection", ({expect, _}) => {
      let selection = collapsedSelection(4);
      let expected =
        notCollapsedSelection(~anchor=4, ~focus=testStringLength, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the end and increase selection", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=4, ~focus=8, ());
      let expected =
        notCollapsedSelection(~anchor=4, ~focus=testStringLength, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the end", ({expect, _}) => {
      let selection =
        notCollapsedSelection(~anchor=5, ~focus=testStringLength, ());
      let expected =
        notCollapsedSelection(~anchor=5, ~focus=testStringLength, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the and and no selection",
      ({expect, _}) => {
      let selection = collapsedSelection(testStringLength);
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(0);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });

    test("Moves cursor to the end and undo selection", ({expect, _}) => {
      let selection =
        notCollapsedSelection(~anchor=testStringLength, ~focus=6, ());
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the end and decrease selection", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=7, ~focus=3, ());
      let expected =
        notCollapsedSelection(~anchor=7, ~focus=testStringLength, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When S-C-LEFT", ({test, _}) => {
    let key = "<S-C-LEFT>";

    test("Moves cursor to previous word boundary", ({expect, _}) => {
      let selection = collapsedSelection(10);
      let expected = notCollapsedSelection(~anchor=10, ~focus=5, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to beginning", ({expect, _}) => {
      let selection = collapsedSelection(3);
      let expected = notCollapsedSelection(~anchor=3, ~focus=0, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the beginning", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=10, ~focus=0, ());
      let expected = notCollapsedSelection(~anchor=10, ~focus=0, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the beginning and no selection",
      ({expect, _}) => {
      let selection = collapsedSelection(0);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });

    test(
      "Moves cursor to the previous word boundary and undo selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=5, ~focus=16, ());
      let expected = notCollapsedSelection(~anchor=5, ~focus=5, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to the previous word boundary and decrease selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=11, ~focus=16, ());
      let expected = notCollapsedSelection(~anchor=11, ~focus=5, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When C-LEFT", ({test, _}) => {
    let key = "<C-LEFT>";

    test("Moves cursor to previous word boundary", ({expect, _}) => {
      let selection = collapsedSelection(10);
      let expected = collapsedSelection(5);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to beginning", ({expect, _}) => {
      let selection = collapsedSelection(3);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the beginning", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=10, ~focus=0, ());
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the beginning and no selection",
      ({expect, _}) => {
      let selection = collapsedSelection(0);
      let expected = collapsedSelection(0);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });

    test(
      "Moves cursor to the previous word boundary and undo selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=5, ~focus=16, ());
      let expected = collapsedSelection(5);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to the previous word boundary and decrease selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=11, ~focus=16, ());
      let expected = collapsedSelection(5);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When S-C-RIGHT", ({test, _}) => {
    let key = "<S-C-RIGHT>";

    test("Moves cursor to next word boundary", ({expect, _}) => {
      let selection = collapsedSelection(10);
      let expected = notCollapsedSelection(~anchor=10, ~focus=16, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to end", ({expect, _}) => {
      let selection = collapsedSelection(44);
      let expected =
        notCollapsedSelection(~anchor=44, ~focus=testStringLength, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the end", ({expect, _}) => {
      let selection =
        notCollapsedSelection(~anchor=10, ~focus=testStringLength, ());
      let expected =
        notCollapsedSelection(~anchor=10, ~focus=testStringLength, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the and and no selection",
      ({expect, _}) => {
      let selection = collapsedSelection(testStringLength);
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });

    test(
      "Moves cursor to the next word boundary and undo selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=16, ~focus=5, ());
      let expected = notCollapsedSelection(~anchor=16, ~focus=16, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to the next word boundary and decrease selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=10, ~focus=6, ());
      let expected = notCollapsedSelection(~anchor=10, ~focus=16, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When C-RIGHT", ({test, _}) => {
    let key = "<C-RIGHT>";

    test("Moves cursor to next word boundary", ({expect, _}) => {
      let selection = collapsedSelection(10);
      let expected = collapsedSelection(16);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to end", ({expect, _}) => {
      let selection = collapsedSelection(44);
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the end", ({expect, _}) => {
      let selection =
        notCollapsedSelection(~anchor=10, ~focus=testStringLength, ());
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Doesn't move cursor position when it at the and and no selection",
      ({expect, _}) => {
      let selection = collapsedSelection(testStringLength);
      let expected = collapsedSelection(testStringLength);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });

    test(
      "Moves cursor to the next word boundary and undo selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=16, ~focus=5, ());
      let expected = collapsedSelection(16);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test(
      "Moves cursor to the next word boundary and decrease selection",
      ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=10, ~focus=6, ());
      let expected = collapsedSelection(16);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When ASCII letter when no selection", ({test, _}) => {
    let key = "F";

    test("Adds character to the beginning", ({expect, _}) => {
      let selection = collapsedSelection(0);
      let expected = collapsedSelection(1);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.string(text).toEqual(
        "FSome interesting. Test. String. Isn't it? Maybe",
      );
      expect.equal(expected, newSelection);
    });

    test("Adds character to the end", ({expect, _}) => {
      let selection = collapsedSelection(testStringLength);
      let expected =
        collapsedSelection(~text=testString ++ key, testStringLength + 1);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(
        "Some interesting. Test. String. Isn't it? MaybeF",
      );
    });

    test("Adds character to the cursor position", ({expect, _}) => {
      let selection = collapsedSelection(7);
      let expected = collapsedSelection(8);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(
        "Some inFteresting. Test. String. Isn't it? Maybe",
      );
    });
  });

  describe("When ASCII letter when with selection", ({test, _}) => {
    let key = "F";

    test("Replaces character", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=0, ~focus=1, ());
      let expected = collapsedSelection(1);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(
        "Fome interesting. Test. String. Isn't it? Maybe",
      );
    });

    test("Adds character many characters", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=16, ~focus=4, ());
      let expected = collapsedSelection(5);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("SomeF. Test. String. Isn't it? Maybe");
    });

    test("Replaces all string", ({expect, _}) => {
      let selection =
        notCollapsedSelection(~anchor=testStringLength, ~focus=0, ());
      let expected = collapsedSelection(1);

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("F");
    });
  });

  describe("When C-a", ({test, _}) => {
    let key = "<C-a>";

    test("Select all when no selection", ({expect, _}) => {
      let selection = collapsedSelection(3);
      let expected =
        notCollapsedSelection(~anchor=0, ~focus=testStringLength, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Select all when is selection", ({expect, _}) => {
      let selection = notCollapsedSelection(~anchor=5, ~focus=24, ());
      let expected =
        notCollapsedSelection(~anchor=0, ~focus=testStringLength, ());

      let (text, newSelection) = runInputHandler(selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual(testString);
    });

    test("Selects nothing with empty string", ({expect, _}) => {
      let selection = collapsedSelection(~text="", 0);
      let expected = collapsedSelection(~text="", 0);

      let (text, newSelection) = runInputHandler(~text="", selection, key);

      expect.equal(expected, newSelection);
      expect.string(text).toEqual("");
    });
  });
});
