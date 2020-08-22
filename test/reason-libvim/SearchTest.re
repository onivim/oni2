open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBrackets = () =>
  Helpers.resetBuffer("test/reason-libvim/brackets.txt");
let reset = () => Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));

let lineNumberToInt = lnum => lnum |> EditorCoreTypes.LineNumber.toZeroBased;
let byteToInt = byte => byte |> ByteIndex.toInt;

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
      expect.int(secondHighlight.start.line |> lineNumberToInt).toBe(1);
      expect.int(secondHighlight.start.byte |> byteToInt).toBe(30);
      expect.int(secondHighlight.stop.line |> lineNumberToInt).toBe(1);
      expect.int(secondHighlight.stop.byte |> byteToInt).toBe(32);
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
});
