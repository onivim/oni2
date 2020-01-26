open TestFramework;

module InputModel = Oni_Components.InputModel;

let testString = "Some interesting. Test. String. Isn't it? Maybe";
let testStringLength = String.length(testString);

let runInputHandler = (~text=testString, position, selection, key) => {
  InputModel.handleInput(
    ~text=text,
    ~cursorPosition=position,
    ~selectionPosition=selection,
  key);
};

describe("InputModel#handleInput", ({describe, _}) => {
  describe("When LEFT with no selection", ({test, _}) => {
    let key = "<LEFT>";

    test("Moves cursor left for 1 character", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(3);
      expect.int(selection).toBe(3);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor less then 0 position", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });
  });

  describe("When LEFT with selection", ({test, _}) => {
    let key = "<LEFT>";

    test("Moves cursor to the beginning of selection when cursor comes first", ({expect}) => {
      let curPosition = 2;
      let selPosition = 4;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(2);
      expect.int(selection).toBe(2);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the beginning of selection when cursor comes last", ({expect}) => {
      let curPosition = 4;
      let selPosition = 2;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(2);
      expect.int(selection).toBe(2);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the beginning of selection when cursor at the beginning", ({expect}) => {
      let curPosition = 0;
      let selPosition = 10;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });
  });

  describe("When <RIGHT> with no selection", ({test, _}) => {
    let key = "<RIGHT>";

    test("Moves cursor right for 1 character", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(5);
      expect.int(selection).toBe(5);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position more that string length", ({expect}) => {
      let position = testStringLength;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });
  });

  describe("When RIGHT with selection", ({test, _}) => {
    let key = "<RIGHT>";

    test("Moves cursor to the end of selection when cursor comes first", ({expect}) => {
      let curPosition = 2;
      let selPosition = 4;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(4);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the end of selection when cursor comes last", ({expect}) => {
      let curPosition = 4;
      let selPosition = 2;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(4);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the end of selection when cursor at the beginning", ({expect}) => {
      let curPosition = 0;
      let selPosition = testStringLength;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });
  });


  describe("When <RIGHT> with no selection", ({test, _}) => {
    let key = "<RIGHT>";

    test("Moves cursor right for 1 character", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(5);
      expect.int(selection).toBe(5);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position more that string length", ({expect}) => {
      let position = testStringLength;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });
  });

  describe("When <BS> with no selection", ({test, _}) => {
    let key = "<BS>";

    test("Removes character on the left of cursor", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(3);
      expect.int(selection).toBe(3);
      expect.string(text).toEqual("Som interesting. Test. String. Isn't it? Maybe");
    });

    test("Doesn't remove character if cursor at the beginnig", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });

    test("Don't do anything for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });
  });

  describe("When <BS> with with selection", ({test, _}) => {
    let key = "<BS>";

    test("Removes selection when cursor comes first", ({expect}) => {
      let curPosition = 2;
      let selPosition = 4;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(2);
      expect.int(selection).toBe(2);
      expect.string(text).toEqual("So interesting. Test. String. Isn't it? Maybe");
    });

    test("Removes selection when cursor comes last", ({expect}) => {
      let curPosition = 4;
      let selPosition = 2;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(2);
      expect.int(selection).toBe(2);
      expect.string(text).toEqual("So interesting. Test. String. Isn't it? Maybe");
    });
  });

  describe("When <DEL> with no selection", ({test, _}) => {
    let key = "<DEL>";

    test("Removes character on the right of cursor", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(4);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual("Someinteresting. Test. String. Isn't it? Maybe");
    });

    test("Doesn't remove character if cursor at the end", ({expect}) => {
      let position = testStringLength;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Don't do anything for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });
  });

  describe("When <DEL> with with selection", ({test, _}) => {
    let key = "<DEL>";

    test("Removes selection when cursor comes first", ({expect}) => {
      let curPosition = 2;
      let selPosition = 4;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(2);
      expect.int(selection).toBe(2);
      expect.string(text).toEqual("So interesting. Test. String. Isn't it? Maybe");
    });

    test("Removes selection when cursor comes last", ({expect}) => {
      let curPosition = 4;
      let selPosition = 2;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(2);
      expect.int(selection).toBe(2);
      expect.string(text).toEqual("So interesting. Test. String. Isn't it? Maybe");
    });
  });

  describe("When HOME with no selection", ({test, _}) => {
    let key = "<HOME>";

    test("Moves cursor to the beginning", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor if it's at the beginning", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });
  });

  describe("When HOME with selection", ({test, _}) => {
    let key = "<HOME>";

    test("Moves cursor to the beginning and discard selection", ({expect}) => {
      let curPosition = 5;
      let selPosition = 11;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor if it's at the beginning and discard selection", ({expect}) => {
      let curPosition = 0;
      let selPosition = 8;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When END with no selection", ({test, _}) => {
    let key = "<END>";

    test("Moves cursor to the end", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor if it's at the end", ({expect}) => {
      let position = testStringLength;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });
  });

  describe("When END with selection", ({test, _}) => {
    let key = "<END>";

    test("Moves cursor to the end and discard selection", ({expect}) => {
      let curPosition = 5;
      let selPosition = 11;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor if it's at the end and discard selection", ({expect}) => {
      let curPosition = 4;
      let selPosition = testStringLength;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When S-LEFT", ({test, _}) => {
    let key = "<S-LEFT>";

    test("Moves cursor to 1 character left and add selection", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(3);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to 1 character left and increase selection", ({expect}) => {
      let curPosition = 5;
      let selPosition = 11;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(4);
      expect.int(selection).toBe(11);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the beginning", ({expect}) => {
      let curPosition = 0;
      let selPosition = 5;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(5);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the beginning and no selection", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });

    test("Moves cursor to 1 character left and undo selection", ({expect}) => {
      let curPosition = 5;
      let selPosition = 4;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(4);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to 1 character left and decrease selection", ({expect}) => {
      let curPosition = 8;
      let selPosition = 4;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(7);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When S-RIGHT", ({test, _}) => {
    let key = "<S-RIGHT>";

    test("Moves cursor to 1 character right and add selection", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(5);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to 1 character right and increase selection", ({expect}) => {
      let curPosition = 11;
      let selPosition = 4;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(12);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the end", ({expect}) => {
      let curPosition = testStringLength;
      let selPosition = 5;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(5);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the end and no selection", ({expect}) => {
      let position = testStringLength;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });

    test("Moves cursor to 1 character right and undo selection", ({expect}) => {
      let curPosition = 5;
      let selPosition = 6;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(6);
      expect.int(selection).toBe(6);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to 1 character right and decrease selection", ({expect}) => {
      let curPosition = 3;
      let selPosition = 8;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(4);
      expect.int(selection).toBe(8);
      expect.string(text).toEqual(testString);
    });
  });


  describe("When S-HOME", ({test, _}) => {
    let key = "<S-HOME>";

    test("Moves cursor to the beginning and add selection", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the beginning and increase selection", ({expect}) => {
      let curPosition = 4;
      let selPosition = 7;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(7);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the beginning", ({expect}) => {
      let curPosition = 0;
      let selPosition = 5;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(5);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the beginning and no selection", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });

    test("Moves cursor to the beginning and undo selection", ({expect}) => {
      let curPosition = 6;
      let selPosition = 0;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the beginning and decrease selection", ({expect}) => {
      let curPosition = 8;
      let selPosition = 3;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(3);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When S-END", ({test, _}) => {
    let key = "<S-END>";

    test("Moves cursor to the end and add selection", ({expect}) => {
      let position = 4;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the end and increase selection", ({expect}) => {
      let curPosition = 8;
      let selPosition = 4;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(4);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the end", ({expect}) => {
      let curPosition = testStringLength;
      let selPosition = 5;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(5);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the and and no selection", ({expect}) => {
      let position = testStringLength;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });

    test("Moves cursor to the end and undo selection", ({expect}) => {
      let curPosition = 6;
      let selPosition = testStringLength;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the end and decrease selection", ({expect}) => {
      let curPosition = 3;
      let selPosition = 7;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(7);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When S-C-LEFT", ({test, _}) => {
    let key = "<S-C-LEFT>";

    test("Moves cursor to previous word boundary", ({expect}) => {
      let position = 10;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(5);
      expect.int(selection).toBe(10);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to beginning", ({expect}) => {
      let position = 3;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(3);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the beginning", ({expect}) => {
      let curPosition = 0;
      let selPosition = 10;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(10);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the beginning and no selection", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });

    test("Moves cursor to the previous word boundary and undo selection", ({expect}) => {
      let curPosition = 16;
      let selPosition = 5;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(5);
      expect.int(selection).toBe(5);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the previous word boundary and decrease selection", ({expect}) => {
      let curPosition = 16;
      let selPosition = 11;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(5);
      expect.int(selection).toBe(11);
      expect.string(text).toEqual(testString);
    });
  });


  describe("When S-C-RIGHT", ({test, _}) => {
    let key = "<S-C-RIGHT>";

    test("Moves cursor to next word boundary", ({expect}) => {
      let position = 10;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(16);
      expect.int(selection).toBe(10);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to end", ({expect}) => {
      let position = 44;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(44);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the end", ({expect}) => {
      let curPosition = testStringLength;
      let selPosition = 10;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(10);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position when it at the and and no selection", ({expect}) => {
      let position = testStringLength;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength);
      expect.int(selection).toBe(testStringLength);
      expect.string(text).toEqual(testString);
    });

    test("Doesn't move cursor position for blank string", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(~text="", position, position, key);

      expect.int(cursor).toBe(0);
      expect.int(selection).toBe(0);
      expect.string(text).toEqual("");
    });

    test("Moves cursor to the next word boundary and undo selection", ({expect}) => {
      let curPosition = 5;
      let selPosition = 16;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(16);
      expect.int(selection).toBe(16);
      expect.string(text).toEqual(testString);
    });

    test("Moves cursor to the next word boundary and decrease selection", ({expect}) => {
      let curPosition = 6;
      let selPosition = 10;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(16);
      expect.int(selection).toBe(10);
      expect.string(text).toEqual(testString);
    });
  });

  describe("When ASCII letter when no selection", ({test, _}) => {
    let key = "F";

    test("Adds character to the beginning", ({expect}) => {
      let position = 0;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(1);
      expect.int(selection).toBe(1);
      expect.string(text).toEqual("FSome interesting. Test. String. Isn't it? Maybe");
    });

    test("Adds character to the end", ({expect}) => {
      let position = testStringLength;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(testStringLength + 1);
      expect.int(selection).toBe(testStringLength + 1);
      expect.string(text).toEqual("Some interesting. Test. String. Isn't it? MaybeF");
    });

    test("Adds character to the cursor position", ({expect}) => {
      let position = 7;

      let (text, cursor, selection) = runInputHandler(position, position, key);

      expect.int(cursor).toBe(8);
      expect.int(selection).toBe(8);
      expect.string(text).toEqual("Some inFteresting. Test. String. Isn't it? Maybe");
    });
  })

  describe("When ASCII letter when with selection", ({test, _}) => {
    let key = "F";

    test("Replaces character", ({expect}) => {
      let curPosition = 1;
      let selPosition = 0;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(1);
      expect.int(selection).toBe(1);
      expect.string(text).toEqual("Fome interesting. Test. String. Isn't it? Maybe");
    });

    test("Adds character many characters", ({expect}) => {
      let curPosition = 4;
      let selPosition = 16;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(5);
      expect.int(selection).toBe(5);
      expect.string(text).toEqual("SomeF. Test. String. Isn't it? Maybe");
    });

    test("Replaces all string", ({expect}) => {
      let curPosition = 0;
      let selPosition = testStringLength;

      let (text, cursor, selection) = runInputHandler(curPosition, selPosition, key);

      expect.int(cursor).toBe(1);
      expect.int(selection).toBe(1);
      expect.string(text).toEqual("F");
    });
  })
});