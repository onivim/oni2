open TestFramework;

module MenuJob = Oni_Model.MenuJob;

describe("MenuJob", ({describe, _}) =>
  describe("regexFromFilter", ({test, _}) =>
    test("regex matches as expected", ({expect, _}) => {
      let regEx = MenuJob.regexFromFilter("abc");
      expect.bool(Str.string_match(regEx, "abc", 0)).toBe(true);
      expect.bool(Str.string_match(regEx, "def", 0)).toBe(false);
      expect.bool(Str.string_match(regEx, "1a1b1c1", 0)).toBe(true);
    })
  )
);
