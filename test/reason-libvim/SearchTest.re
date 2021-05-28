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
  describe("getSearchPattern", ({test, _}) => {
    test("search with /", ({expect, _}) => {
      let _ = reset();
      let _ = Vim.command("nohl");

      input("/");
      input("e");

      expect.equal(Search.getSearchPattern(), Some("e"));

      input("f");

      expect.equal(Search.getSearchPattern(), Some("ef"));
    });

    test("search with ?", ({expect, _}) => {
      let _ = reset();
      let _ = Vim.command("nohl");

      input("?");
      input("a");

      expect.equal(Search.getSearchPattern(), Some("a"));

      input("b");

      expect.equal(Search.getSearchPattern(), Some("ab"));
    });

    test("search with /, unicode", ({expect, _}) => {
      let _ = reset();
      let _ = Vim.command("nohl");

      input("?");
      input("κ");

      expect.equal(Search.getSearchPattern(), Some("κ"));

      input("ό");

      expect.equal(Search.getSearchPattern(), Some("κό"));
    });
  });

  describe("getSearchHighlights", ({test, _}) => {
    test("gets highlights", ({expect, _}) => {
      let _ = reset();

      input("/");
      input("e");

      let highlights = Search.getHighlights(Vim.Buffer.getCurrent());
      expect.int(Array.length(highlights)).toBe(13);

      input("s");
      let highlights = Search.getHighlights(Vim.Buffer.getCurrent());
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

      let highlights =
        Search.getHighlightsInRange(Vim.Buffer.getCurrent(), 1, 1);
      expect.int(Array.length(highlights)).toBe(4);

      input("s");
      let highlights =
        Search.getHighlightsInRange(Vim.Buffer.getCurrent(), 1, 1);
      expect.int(Array.length(highlights)).toBe(1);
    });
  });

  describe("SearchClearHighlights", ({test, _}) =>
    test(
      "SearchClearHighlights effect is produced when nohlsearch is called",
      ({expect, _}) => {
      let _ = reset();

      let (_context: Context.t, effects: list(Vim.Effect.t)) =
        Vim.command("nohlsearch");

      let hasEffect =
        effects |> List.exists(eff => eff == Vim.Effect.SearchClearHighlights);
      expect.bool(hasEffect).toBe(true);
    })
  );
});
