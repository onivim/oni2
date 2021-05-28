open Vterm;
open TestFramework;

describe("VTerm", ({describe, test, _}) => {
  test("make", ({expect, _}) => {
    let isFinalized = ref(false);
    let vterm = make(~rows=20, ~cols=20);

    Gc.finalise(_ => isFinalized := true, vterm);
    Gc.full_major();

    expect.equal(isFinalized^, true);
  });
  test("utf8", ({expect, _}) => {
    let vterm = make(~rows=20, ~cols=20);
    setUtf8(~utf8=true, vterm);
    expect.equal(getUtf8(vterm), true);
    setUtf8(~utf8=false, vterm);
    expect.equal(getUtf8(vterm), false);
  });
  test("size", ({expect, _}) => {
    let vterm = make(~rows=20, ~cols=30);
    let {rows, cols} = getSize(vterm);
    expect.equal(rows, 20);
    expect.equal(cols, 30);

    setSize(~size={rows: 10, cols: 15}, vterm);
    let {rows, cols} = getSize(vterm);
    expect.equal(rows, 10);
    expect.equal(cols, 15);
  });
  describe("screen", ({test, _}) => {
    test("gets empty cell", ({expect, _}) => {
      let vterm = make(~rows=20, ~cols=30);
      let cell = Screen.getCell(~row=0, ~col=0, vterm);
      expect.equal(cell.char |> Uchar.to_char, Char.chr(0));
    });
    test("gets non-empty cell", ({expect, _}) => {
      let vterm = make(~rows=20, ~cols=30);
      let _: int = write(~input=String.make(1, 'a'), vterm);
      let cell = Screen.getCell(~row=0, ~col=0, vterm);
      expect.equal(cell.char |> Uchar.to_char, 'a');
    });
  });
  describe("input", ({test, describe, _}) => {
    describe("unicode", ({test, _}) => {
      test("replacement character round-trips", ({expect, _}) => {
        let vterm = make(~rows=20, ~cols=30);
        Vterm.setUtf8(~utf8=true, vterm);

        let damageCount = ref(0);
        Screen.setDamageCallback(~onDamage=_ => incr(damageCount), vterm);

        let buffer = Buffer.create(5);
        Buffer.add_utf_8_uchar(buffer, Uchar.rep);
        let str = Buffer.contents(buffer);

        // Write Unicode replacement character
        let _: int = write(~input=str, vterm);

        // ...and verify it roundtrips
        let cell = Screen.getCell(~row=0, ~col=0, vterm);
        expect.equal(cell.char, Uchar.rep);
      })
    });
    describe("TermProps", ({test, _}) => {
      test("title term prop is set", ({expect, _}) => {
        let vterm = make(~rows=20, ~cols=30);

        let title = ref("");
        Screen.setTermPropCallback(
          ~onSetTermProp=
            fun
            | TermProp.Title(t) => title := t
            | _ => (),
          vterm,
        );

        let str = "\027]2;abc";
        let _: int = write(~input=str, vterm);

        expect.equal(title^, "abc");
      });
      test("icon term prop is set", ({expect, _}) => {
        let vterm = make(~rows=20, ~cols=30);

        let icon = ref("");
        Screen.setTermPropCallback(
          ~onSetTermProp=
            fun
            | TermProp.IconName(t) => icon := t
            | _ => (),
          vterm,
        );

        let str = "\027]1;icon";
        let _: int = write(~input=str, vterm);

        expect.equal(icon^, "icon");
      });
    });
    test("returns value", ({expect, _}) => {
      let vterm = make(~rows=20, ~cols=30);

      let res = write(~input="abc", vterm);
      expect.equal(res, 3);
    });

    test("beeps", ({expect, _}) => {
      let vterm = make(~rows=20, ~cols=30);

      let gotBell = ref(false);
      Screen.setBellCallback(~onBell=_ => gotBell := true, vterm);
      let _: int = write(~input=String.make(1, Char.chr(7)), vterm);
      expect.equal(true, gotBell^);
    });
    test("resize", ({expect, _}) => {
      let vterm = make(~rows=20, ~cols=30);

      let getRows = ref(0);
      let getCols = ref(0);
      Screen.setResizeCallback(
        ~onResize=
          ({rows, cols}) => {
            getRows := rows;
            getCols := cols;
          },
        vterm,
      );
      setSize(~size={rows: 5, cols: 6}, vterm);
      expect.equal(getRows^, 5);
      expect.equal(getCols^, 6);
    });

    test("damage", ({expect, _}) => {
      let vterm = make(~rows=20, ~cols=30);

      let damageCount = ref(0);
      Screen.setDamageCallback(~onDamage=_ => incr(damageCount), vterm);

      let _: int = write(~input="a", vterm);
      expect.equal(damageCount^, 1);

      Gc.full_major();
      let _: int = write(~input="b", vterm);
      expect.equal(damageCount^, 2);
    });
    test("regression test: corruption with GC", ({expect, _}) => {
      let size = 25;
      // This reproduces an issue where we had grabbed a char* pointer
      // in the C binding, but during the course of operation, when the GC
      // kicked in - that pointer would no longer be valid, and we'd end up
      // with corrupted output.
      let vterm = make(~rows=size, ~cols=size);

      // We can reproduce this by writing a long string, and forcing a GC.
      Screen.setDamageCallback(~onDamage=_ => Gc.full_major(), vterm);

      let longString = String.make(size * size * 2, 'a');
      let _: int = write(~input=longString, vterm);

      // Validate all cells show 'a'
      for (row in 0 to size - 1) {
        for (col in 0 to size - 1) {
          let cell = Screen.getCell(~row, ~col, vterm);
          expect.equal(cell.char |> Uchar.to_char, 'a');
        };
      };
    });
  });
  describe("output", ({test, _}) => {
    let str = i => i |> Char.chr |> String.make(1);
    test("Unicode character", ({expect, _}) => {
      let vterm = make(~rows=20, ~cols=30);

      let output = ref([]);
      setOutputCallback(~onOutput=s => output := [s], vterm);
      let () = Keyboard.input(vterm, Unicode(Uchar.of_int(65)), None);
      expect.equal(["A"], output^);
    });
    test("Enter character", ({expect, _}) => {
      let vterm = make(~rows=20, ~cols=30);

      let output = ref([]);
      setOutputCallback(~onOutput=s => output := [s], vterm);
      let () = Keyboard.input(vterm, Enter, None);
      expect.equal([str(13)], output^);
    });
  });
});
