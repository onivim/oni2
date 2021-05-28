open TestFramework;

module Theme = Textmate.Theme;
module ColorTheme = Textmate.ColorTheme;
module TokenTheme = Textmate.TokenTheme;
module Scope = Textmate.ThemeScopes.Scope;
module Selector = Textmate.ThemeScopes.Selector;
module ResolvedStyle = Textmate.ThemeScopes.ResolvedStyle;
module TokenStyle = Textmate.ThemeScopes.TokenStyle;

let resultValue =
  fun
  | Ok(v) => v
  | Error(msg) => {
      print_endline(msg);
      failwith("resultValue");
    };

// INVALID

describe("invalid", ({test, _}) => {
  test("should give error", ({expect, _}) => {
    let loadResult =
      Theme.from_file("test/reason-textmate/onivim/fixtures/invalid.json");
    expect.equal(true, Result.is_error(loadResult));
  })
});

// ONE DARK

describe("OneDark", ({test, _}) => {
  let oneDark =
    Theme.from_file("test/reason-textmate/onivim/fixtures/OneDark-Pro.json")
    |> resultValue;
  let oneDarkLight =
    Theme.from_file(
      "test/reason-textmate/onivim/fixtures/OneDarkPro-Light.json",
    )
    |> resultValue;
  let oneDarkTheme = Theme.getTokenColors(oneDark);
  let oneDarkColors = Theme.getColors(oneDark);

  let darkPlus =
    Theme.from_file("test/reason-textmate/onivim/fixtures/dark_plus.json")
    |> resultValue;
  let darkPlusTheme = Theme.getTokenColors(darkPlus);
  let darkPlusColors = Theme.getColors(darkPlus);

  test("OneDark-Pro: isDark", ({expect, _}) => {
    expect.bool(Theme.isDark(oneDark)).toBe(true)
  });
  test("OneDark-Light: isDark", ({expect, _}) => {
    expect.bool(Theme.isDark(oneDarkLight)).toBe(false)
  });

  test("dark_plus - colors: load nested", ({expect, _}) => {
    // Load a color that is _only_ in parent
    switch (ColorTheme.getColor("editor.background", darkPlusColors)) {
    | None => expect.int(0).toBe(1)
    | Some(v) => expect.string(v).toEqual("#1E1E1E")
    };

    // Load a color that is _overridden_
    switch (
      ColorTheme.getColor(
        "editor.inactiveSelectionBackground",
        darkPlusColors,
      )
    ) {
    | None => expect.int(0).toBe(1)
    | Some(v) => expect.string(v).toEqual("#AABBCC")
    };
  });

  test("dark_plus - tokenColors: load nested", ({expect, _}) => {
    // Load a token that is _only_ available in parent
    let token = TokenTheme.match(darkPlusTheme, "comment");
    expect.string(token.foreground).toEqual("#6A9955");

    // Load a token that is _overridden_
    let token = TokenTheme.match(darkPlusTheme, "entity.name.label");
    expect.string(token.foreground).toEqual("#C9C9C9");
  });

  test("colors: activityBar.background", ({expect, _}) => {
    switch (ColorTheme.getColor("activityBar.background", oneDarkColors)) {
    | None => expect.int(0).toBe(1)
    | Some(v) => expect.string(v).toEqual("#282c34")
    }
  });

  test("matches multiple scopes", ({expect, _}) => {
    let token =
      TokenTheme.match(
        oneDarkTheme,
        "source.reason markup.inserted constant.language support.property-value entity.name.filename",
      );
    expect.string(token.foreground).toEqual("#d19a66");
  });

  test("c: matches include", ({expect, _}) => {
    let token =
      TokenTheme.match(
        oneDarkTheme,
        "source.c meta.preprocessor.include.c keyword.control.directive.$3.c",
      );
    expect.string(token.foreground).toEqual("#c678dd");
  });

  test("c: matches include punctuation ('#')", ({expect, _}) => {
    prerr_endline("============ BEGIN ===============");
    let token =
      TokenTheme.match(
        oneDarkTheme,
        "source.c meta.preprocessor.include.c keyword.control.directive.$3.c punctuation.definition.directive.c",
      );
    expect.string(token.foreground).toEqual("#c678dd");
    prerr_endline("============ END ===============");
  });
});

// XML

describe("XML", ({describe, _}) => {
  describe("Duotone", ({test, _}) => {
    let theme =
      Theme.from_file(
        ~isDark=true,
        "test/reason-textmate/onivim/fixtures/duotone-earth.xml",
      )
      |> resultValue;

    let colors = Theme.getColors(theme);
    let tokenColors = Theme.getTokenColors(theme);

    test("isDark", ({expect, _}) => {
      expect.bool(Theme.isDark(theme)).toBe(true)
    });

    test("colors: selection", ({expect, _}) => {
      let actual = ColorTheme.getColor("selection", colors);
      expect.equal(actual, Some("#4D4642"));
    });

    test("tokenColors: support.type", ({expect, _}) => {
      let token = TokenTheme.match(tokenColors, "support.type");
      expect.string(token.foreground).toEqual("#98755d");
    });
  });

  describe("Tomorrow Operator Mono", ({test, _}) => {
    let theme =
      Theme.from_file(
        ~isDark=true,
        "test/reason-textmate/onivim/fixtures/tomorrow-operator-mono.xml",
      )
      |> resultValue;

    let colors = Theme.getColors(theme);
    let tokenColors = Theme.getTokenColors(theme);

    test("isDark", ({expect, _}) => {
      expect.bool(Theme.isDark(theme)).toBe(true)
    });

    test("colors: selection", ({expect, _}) => {
      let actual = ColorTheme.getColor("selection", colors);
      expect.equal(actual, Some("#D6D6D6"));
    });

    test("tokenColors: support.type", ({expect, _}) => {
      let token = TokenTheme.match(tokenColors, "support.type");
      expect.string(token.foreground).toEqual("#C99E00");
    });
  });
});
