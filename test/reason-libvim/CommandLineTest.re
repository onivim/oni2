open EditorCoreTypes;
open Vim;
open TestFramework;

let reset = () => Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));
let key = s => ignore(Vim.key(s));

describe("CommandLine", ({describe, _}) => {
  let context = colorSchemeProvider => {
    ...Vim.Context.current(),
    colorSchemeProvider,
  };
  let getCompletions = CommandLine.getCompletions;
  describe("getType", ({test, _}) =>
    test("simple command line", ({expect, _}) => {
      let _ = reset();
      let ({mode, _}: Vim.Context.t, _) = Vim.input(":");

      let getType =
        fun
        | Vim.Mode.CommandLine({commandType, _}) => Some(commandType)
        | _ => None;

      expect.equal(getType(mode), Some(Types.Ex));
      key("<esc>");

      let ({mode, _}: Vim.Context.t, _) = Vim.input("/");
      expect.equal(getType(mode), Some(Types.SearchForward));
      key("<esc>");

      let ({mode, _}: Vim.Context.t, _) = Vim.input("?");
      expect.equal(getType(mode), Some(Types.SearchReverse));
      key("<esc>");
    })
  );

  describe("getCompletions", ({describe, test, _}) => {
    describe("ColorSchemes", ({test, _}) => {
      test("gc stress test", ({expect, _}) => {
        let _ = reset();

        input(":colorscheme ");
        let colorSchemeProvider = _ => {
          Gc.compact();
          Array.make(10000, String.make(1000, 'a'));
        };

        expect.int(
          Array.length(
            CommandLine.getCompletions(
              ~context=context(colorSchemeProvider),
              (),
            ),
          ),
        ).
          toBe(
          10000,
        );
      });
      test("pattern gets set", ({expect, _}) => {
        let _ = reset();

        let lastPattern = ref(None);
        input(":colorscheme ");
        let colorSchemeProvider = pattern => {
          lastPattern := Some(pattern);
          [|"a"|];
        };

        expect.int(
          Array.length(
            CommandLine.getCompletions(
              ~context=context(colorSchemeProvider),
              (),
            ),
          ),
        ).
          toBe(
          1,
        );
        expect.equal(lastPattern^, Some(""));

        input("a");
        expect.int(
          Array.length(
            CommandLine.getCompletions(
              ~context=context(colorSchemeProvider),
              (),
            ),
          ),
        ).
          toBe(
          1,
        );
        expect.equal(lastPattern^, Some("a"));

        input("b");
        expect.int(
          Array.length(
            CommandLine.getCompletions(
              ~context=context(colorSchemeProvider),
              (),
            ),
          ),
        ).
          toBe(
          1,
        );
        expect.equal(lastPattern^, Some("ab"));
      });
    });
    test("basic completion test", ({expect, _}) => {
      let _ = reset();
      input(":");
      input("e");

      expect.int(Array.length(getCompletions())).toBe(20);
    });

    test("request completions multiple times", ({expect, _}) => {
      let _ = reset();
      input(":");
      input("e");

      expect.int(Array.length(getCompletions())).toBe(20);
      expect.int(Array.length(getCompletions())).toBe(20);

      let completions = getCompletions();
      expect.string(completions[0]).toEqual("earlier");
    });

    test("regression test - eh", ({expect, _}) => {
      let _ = reset();
      input(":");
      input("e");
      input("h");

      expect.int(Array.length(getCompletions())).toBe(0);
    });
  });

  describe("listeners", ({test, _}) => {
    test("enter / leave listeners", ({expect, _}) => {
      open Vim.Types;
      let _ = reset();
      let enterEvents: ref(list(CommandLine.t)) = ref([]);
      let getEnterCount = () => List.length(enterEvents^);
      let leaveCount = ref(0);

      let expectCommandLineType = m =>
        expect.bool(m == List.hd(enterEvents^).cmdType).toBe(true);
      let expectCommandLineText = v =>
        expect.bool(String.equal(v, List.hd(enterEvents^).text)).toBe(
          true,
        );

      let dispose1 =
        CommandLine.onEnter(c => enterEvents := [c, ...enterEvents^]);
      let dispose2 = CommandLine.onLeave(_ => incr(leaveCount));

      input(":");
      expect.int(getEnterCount()).toBe(1);
      expect.int(leaveCount^).toBe(0);
      expectCommandLineType(Ex);
      expectCommandLineText("");
      input("a");
      expect.int(getEnterCount()).toBe(1);
      expect.int(leaveCount^).toBe(0);
      key("<esc>");
      expect.int(getEnterCount()).toBe(1);
      expect.int(leaveCount^).toBe(1);

      input("/");
      expectCommandLineType(SearchForward);
      expectCommandLineText("");
      expect.int(getEnterCount()).toBe(2);
      expect.int(leaveCount^).toBe(1);
      input("b");
      expect.int(getEnterCount()).toBe(2);
      expect.int(leaveCount^).toBe(1);
      key("<esc>");
      expect.int(getEnterCount()).toBe(2);
      expect.int(leaveCount^).toBe(2);

      dispose1();
      dispose2();
    });

    test("update listener", ({expect, _}) => {
      open Vim.Types;
      let _ = reset();

      let updateEvents: ref(list(CommandLine.t)) = ref([]);

      let getUpdateCount = () => List.length(updateEvents^);
      let expectCommandLineText = v =>
        expect.bool(String.equal(v, List.hd(updateEvents^).text)).toBe(
          true,
        );
      let expectCommandLinePosition = v =>
        expect.bool(v == List.hd(updateEvents^).position).toBe(true);

      let dispose1 =
        CommandLine.onUpdate(c => updateEvents := [c, ...updateEvents^]);

      input(":");
      input("a");
      expect.int(getUpdateCount()).toBe(1);
      expectCommandLineText("a");
      expectCommandLinePosition(1);
      input("b");
      expectCommandLineText("ab");
      expectCommandLinePosition(2);
      key("<c-h>");
      expectCommandLineText("a");
      expectCommandLinePosition(1);

      key("<esc>");
      dispose1();
    });
  });

  describe("ex", ({test, _}) => {
    test("substitution command", ({expect, _}) => {
      let buffer = reset();
      input(":");
      input("%");
      input("s");
      input("!");
      input("T");
      input("!");
      input("D");
      input("!");
      input("g");
      key("<cr>");

      expect.bool(Vim.Mode.isNormal(Mode.current())).toBe(true);

      let line = Buffer.getLine(buffer, LineNumber.ofOneBased(3));
      expect.string(line).toEqual("Dhis is the third line of a test file");
    });

    test("move to line", ({expect, _}) => {
      let _ = reset();
      input(":");
      input("3");
      key("<cr>");

      expect.int(Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased).
        toBe(
        2,
      );
    });

    test(":intro", ({expect, _}) => {
      let _ = reset();
      let hit = ref(false);

      let _: Event.unsubscribe = Vim.onIntro(() => hit := true);
      input(":");
      input("intro");
      key("<cr>");

      expect.equal(hit^, true);
    });

    test(":version", ({expect, _}) => {
      let _ = reset();
      let hit = ref(false);

      let _: Event.unsubscribe = Vim.onVersion(() => hit := true);
      input(":");
      input("version");
      key("<cr>");

      expect.equal(hit^, true);
    });
  });

  describe("getText", ({test, _}) =>
    test("simple command line", ({expect, _}) => {
      let _ = reset();

      input(":");
      let ({mode, _}: Vim.Context.t, _) = Vim.input("a");

      let getText =
        fun
        | Vim.Mode.CommandLine({text, _}) => Some(text)
        | _ => None;

      expect.equal(getText(mode), Some("a"));

      let ({mode, _}: Vim.Context.t, _) = Vim.input("b");
      expect.equal(getText(mode), Some("ab"));

      let ({mode, _}: Vim.Context.t, _) = Vim.input("c");
      expect.equal(getText(mode), Some("abc"));
    })
  );

  describe("getPosition", ({test, _}) =>
    test("simple command line", ({expect, _}) => {
      let _ = reset();
      input(":");
      expect.int(CommandLine.getPosition()).toBe(0);

      input("a");
      expect.int(CommandLine.getPosition()).toBe(1);

      input("b");
      expect.int(CommandLine.getPosition()).toBe(2);

      input("c");
      expect.int(CommandLine.getPosition()).toBe(3);
    })
  );
});
