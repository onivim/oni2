open EditorCoreTypes;
open Vim;
open TestFramework;
open Range;

let resetBrackets = () => Helpers.resetBuffer("test/reason-libvim/brackets.txt");
let reset = () => Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("Search", ({describe, _}) => {
  describe("getSearchHighlights", ({test, _}) => {
    test("gets highlights", ({expect, _}) => {
      let _ = reset();

      input("/");
      input("e");

      let highlights = Search.getHighlights();
      expect.int(Array.length(highlights)).toBe(13);

      input("s");
      let highlights = Search.getHighlights();
      expect.int(Array.length(highlights)).toBe(3);

      let secondHighlight = highlights[1];
      expect.int((secondHighlight.start.line :> int)).toBe(1);
      expect.int((secondHighlight.start.column :> int)).toBe(30);
      expect.int((secondHighlight.stop.line :> int)).toBe(1);
      expect.int((secondHighlight.stop.column :> int)).toBe(32);
    });

    test("gets highlights", ({expect, _}) => {
      let _ = reset();

      input("/");
      input("e");

      let highlights = Search.getHighlightsInRange(1, 1);
      expect.int(Array.length(highlights)).toBe(4);

      input("s");
      let highlights = Search.getHighlightsInRange(1, 1);
      expect.int(Array.length(highlights)).toBe(1);
    });
  });

  describe("onStopSearchHighlight", ({test, _}) =>
    test(
      "onStopSearchHighlight dispatches when nohlsearch is called",
      ({expect, _}) => {
      let _ = reset();

      let callCount = ref(0);
      let unsubscribe =
        Vim.Search.onStopSearchHighlight(() => incr(callCount));

      let _context: Context.t = Vim.command("nohlsearch");

      expect.int(callCount^).toBe(1);

      unsubscribe();
    })
  );

  describe("getMatchingPair", ({test, _}) => {
    test("get matching bracket for initial character", ({expect, _}) => {
      let _ = resetBrackets();

      let bracket = Search.getMatchingPair();
      switch (bracket) {
      | None => expect.int(0).toBe(1)
      | Some({line, column}) =>
        expect.int((line :> int)).toBe(5);
        expect.int((column :> int)).toBe(0);
      };
    });

    test("get matching bracket after moving", ({expect, _}) => {
      let _ = resetBrackets();

      input("l");

      let bracket = Search.getMatchingPair();
      switch (bracket) {
      | None => expect.int(0).toBe(1)
      | Some({line, column}) =>
        expect.int((line :> int)).toBe(3);
        expect.int((column :> int)).toBe(0);
      };
    });

    test("matching bracket is none when there is no match", ({expect, _}) => {
      let _ = resetBrackets();

      input("j");

      let bracket = Search.getMatchingPair();
      switch (bracket) {
      | None => expect.int(1).toBe(1)
      | Some(_) => expect.int(0).toBe(1)
      };
    });
  });
});
