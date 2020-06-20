open TestFramework;

open Oniguruma;
module Match = OnigRegExp.Match;

describe("OnigRegExp", ({describe, _}) => {
  describe("stress", ({test, _}) => {
    test("heavy allocations", ({expect, _}) => {
      let count = 100000;
      let array = Array.make(count, None);

      for (i in 0 to count - 1) {
        let r =
          OnigRegExp.create("abcdefghijklmnopqrstuvwxyz" ++ string_of_int(i))
          |> Result.get_ok;
        array[i] = Some(r);
      };

      for (i in 0 to count - 1) {
        let regexp = array[i] |> Option.get;

        expect.equal(
          OnigRegExp.test(
            "abcdefghijklmnopqrstuvwxyz" ++ string_of_int(i),
            regexp,
          ),
          true,
        );
      };
    })
  });
  describe("fast", ({test, _}) => {
    test("test", ({expect, _}) => {
          let regex =  OnigRegExp.create("abc")
          |> Result.get_ok;
        expect.equal(OnigRegExp.Fast.test("abc", regex), true);
        expect.equal(OnigRegExp.Fast.test("def", regex), false);
    })
    test("capture group test - multiple runs", ({expect, _}) => {
      let r = OnigRegExp.create("(@selector\\()(.*?)(\\))");
      switch (r) {
      | Error(_) => expect.string("Fail").toEqual("")
      | Ok(regex) =>
        // Run regexp with match group multiple times, verify we aren't leaking OnigRegions
        let idx = ref(0);
        while (idx^ < 5) {
          let str = "@selector(windowWillClose:)";
          let _: int =
            OnigRegExp.Fast.search(str, 0, regex);

          let result = OnigRegExp.Fast.getLastMatches(str, regex);
          expect.string(Match.getText(result[1])).toEqual("@selector(");
          expect.string(Match.getText(result[3])).toEqual(")");
          incr(idx);
        };
      };
    });
    describe("stress", ({test, _}) => {
      test("heavy allocations", ({expect, _}) => {
        let count = 100000;
        let array = Array.make(count, None);

        for (i in 0 to count - 1) {
          let r =
            OnigRegExp.create("abcdefghijklmnopqrstuvwxyz" ++ string_of_int(i))
            |> Result.get_ok;
          array[i] = Some(r);
        };

        for (i in 0 to count - 1) {
          let regexp = array[i] |> Option.get;

          expect.equal(
            OnigRegExp.Fast.test(
              "abcdefghijklmnopqrstuvwxyz" ++ string_of_int(i),
              regexp,
            ),
            true,
          );

          let matches = OnigRegExp.Fast.getLastMatches(
              "abcdefghijklmnopqrstuvwxyz" ++ string_of_int(i),
              regexp,
          );
          expect.equal(Array.length(matches), 1);
        };
      })
    });
  });
  describe("allocation", ({test, _}) => {
    test("finalizer gets called for regexp", ({expect, _}) => {
      let regexp = OnigRegExp.create("\\w(\\d+)");
      let callCount = ref(0);
      Gc.finalise_last(() => incr(callCount), regexp);
      Gc.full_major();

      expect.int(callCount^).toBe(1);
    })
  });
  describe("create", ({test, _}) => {
    test("erronous regexp - never ending recursion", ({expect, _}) => {
      let r = OnigRegExp.create("?<abc>\\g<abc>");
      switch (r) {
      | Ok(_) => expect.int(0).toBe(1)
      | Error(msg) =>
        expect.string(msg).toEqual(
          "target of repeat operator is not specified",
        )
      };
    });
    test("erronous regexp - undefined callout", ({expect, _}) => {
      let r = OnigRegExp.create("(*FOO)");
      switch (r) {
      | Ok(_) => expect.int(0).toBe(1)
      | Error(msg) => expect.string(msg).toEqual("undefined callout name")
      };
    });
  });
  describe("test", ({test, _}) => {
    test("increase indent pattern", ({expect, _}) => {
      let r =
        OnigRegExp.create(
          "^((?!\\/\\/).)*(\\{[^}\"'`]*|\\([^)\"'`]*|\\[[^\\]\"'`]*)$",
        );

      switch (r) {
      | Error(_) => expect.string("Fail").toEqual("")
      | Ok(regex) =>
        expect.bool(OnigRegExp.test("{", regex)).toBe(true);
        expect.bool(OnigRegExp.test("abc", regex)).toBe(false);
      };
    })
  });
  describe("search", ({test, _}) => {
    test("comment regex", ({expect, _}) => {
      let r = OnigRegExp.create("(//).*$\\n?");

      switch (r) {
      | Error(_) => expect.string("Fail").toEqual("")
      | Ok(regex) =>
        let result = OnigRegExp.search("// abc", 0, regex);
        expect.int(Array.length(result)).toBe(2);
        expect.string(Match.getText(result[0])).toEqual("// abc");
        expect.string(Match.getText(result[1])).toEqual("//");
      };
    });

    test("returns empty array if it does not match", ({expect, _}) => {
      let r = OnigRegExp.create("\\w(\\d+)");
      switch (r) {
      | Error(_) => expect.string("Fail").toEqual("")
      | Ok(regex) =>
        let result = OnigRegExp.search("-----------", 0, regex);
        expect.int(Array.length(result)).toBe(0);
      };
    });
    test("unicode character", ({expect, _}) => {
      let r = OnigRegExp.create("a");
      switch (r) {
      | Error(_) => expect.string("fail").toEqual("")
      | Ok(regex) =>
        let result = OnigRegExp.search("ç√Ωa", 0, regex);
        expect.int(Array.length(result)).toBe(1);
        expect.int(result[0].startPos).toBe(7);
        expect.string(Match.getText(result[0])).toEqual("a");
      };
    });
    test("match unicode character", ({expect, _}) => {
      let r = OnigRegExp.create("√");
      switch (r) {
      | Error(_) => expect.string("fail").toEqual("")
      | Ok(regex) =>
        let result = OnigRegExp.search("ç√Ωa", 0, regex);
        expect.int(Array.length(result)).toBe(1);
        expect.int(result[0].startPos).toBe(2);
        expect.string(Match.getText(result[0])).toEqual("√");
      };
    });
    test("returns regions if it does match", ({expect, _}) => {
      let r = OnigRegExp.create("\\w(\\d+)");
      switch (r) {
      | Error(_) => expect.string("Fail").toEqual("")
      | Ok(regex) =>
        let result = OnigRegExp.search("----a123---", 0, regex);
        expect.int(Array.length(result)).toBe(2);
        expect.int(result[0].startPos).toBe(4);
        expect.int(result[0].endPos).toBe(8);
        expect.int(result[0].index).toBe(0);
        expect.string(Match.getText(result[0])).toEqual("a123");
        expect.int(result[0].length).toBe(4);
        expect.int(result[1].startPos).toBe(5);
        expect.int(result[1].endPos).toBe(8);
        expect.int(result[1].index).toBe(1);
        expect.int(result[1].length).toBe(3);
        expect.string(Match.getText(result[1])).toEqual("123");
      };
    });
    test("capture group test", ({expect, _}) => {
      let r = OnigRegExp.create("(@selector\\()(.*?)(\\))");
      switch (r) {
      | Error(_) => expect.string("Fail").toEqual("")
      | Ok(regex) =>
        let result =
          OnigRegExp.search("@selector(windowWillClose:)", 0, regex);
        expect.string(Match.getText(result[1])).toEqual("@selector(");
        expect.string(Match.getText(result[3])).toEqual(")");
      };
    });
    test("capture group test - multiple runs", ({expect, _}) => {
      let r = OnigRegExp.create("(@selector\\()(.*?)(\\))");
      switch (r) {
      | Error(_) => expect.string("Fail").toEqual("")
      | Ok(regex) =>
        // Run regexp with match group multiple times, verify we aren't leaking OnigRegions
        let idx = ref(0);
        while (idx^ < 5) {
          let result =
            OnigRegExp.search("@selector(windowWillClose:)", 0, regex);
          expect.string(Match.getText(result[1])).toEqual("@selector(");
          expect.string(Match.getText(result[3])).toEqual(")");
          incr(idx);
        };
      };
    });
  });
});
