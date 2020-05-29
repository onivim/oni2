open TestFramework;

module TokenTheme = Textmate.TokenTheme;
module Scope = Textmate.ThemeScopes.Scope;
module Selector = Textmate.ThemeScopes.Selector;
module ResolvedStyle = Textmate.ThemeScopes.ResolvedStyle;
module TokenStyle = Textmate.ThemeScopes.TokenStyle;

describe("TokenTheme", ({describe, _}) => {
  /* Test theme inspired by:
        https://code.visualstudio.com/blogs/2017/02/08/syntax-highlighting-optimizations#_finally-whats-new-in-vs-code-19
     */

  let simpleTokenTheme =
    TokenTheme.create(
      ~defaultBackground="#000",
      ~defaultForeground="#fff",
      [
        ("var", TokenStyle.create(~foreground=Some("#92D3CA"), ())),
        (
          "var.identifier",
          TokenStyle.create(
            ~foreground=Some("#007fff"),
            ~bold=Some(true),
            (),
          ),
        ),
        (
          "constant",
          TokenStyle.create(
            ~foreground=Some("#00FFFF"),
            ~italic=Some(true),
            (),
          ),
        ),
        (
          "constant.numeric",
          TokenStyle.create(~foreground=Some("#990000"), ()),
        ),
        ("constant.numeric.hex", TokenStyle.create(~bold=Some(true), ())),
        ("foo, bar", TokenStyle.create(~foreground=Some("lavender"), ())),
        ("entity", TokenStyle.create(~bold=Some(true), ())),
        (
          "entity.other.attribute-name.foo,entity.other.attribute-name.bar",
          TokenStyle.create(~foreground=Some("salmon"), ()),
        ),
        ("html", TokenStyle.create(~foreground=Some("slateGray"), ())),
        ("meta html", TokenStyle.create(~foreground=Some("smoke"), ())),
        (
          "source.php string",
          TokenStyle.create(~foreground=Some("peachPuff"), ()),
        ),
        (
          "text.html source.php",
          TokenStyle.create(~foreground=Some("navy"), ()),
        ),
        (
          "text.html source.js",
          TokenStyle.create(
            ~foreground=Some("navy"),
            ~background=Some("cornflowerBlue"),
            (),
          ),
        ),
      ],
    );

  describe("match", ({test, _}) => {
    test("superfluous styles should still match", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(
          simpleTokenTheme,
          "source.js constant.numeric.meta.js",
        );

      expect.string(style.foreground).toEqual("#990000");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });
    test("unmatched style should pass-through", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(
          simpleTokenTheme,
          "source.js constant.numeric.meta.js some-unmatched-style",
        );

      expect.string(style.foreground).toEqual("#990000");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });
    test("background color should be picked up", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(simpleTokenTheme, "text.html.basic source.js");

      expect.string(style.foreground).toEqual("navy");
      expect.string(style.background).toEqual("cornflowerBlue");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test(
      "deeper rule should win (source.php string over text.html source.php)",
      ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(
          simpleTokenTheme,
          "text.html.basic source.php.html string.quoted",
        );

      expect.string(style.foreground).toEqual("peachPuff");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("parent rule (meta html) gets applied", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(simpleTokenTheme, "meta html");

      expect.string(style.foreground).toEqual("smoke");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);

      let style: ResolvedStyle.t =
        TokenTheme.match(simpleTokenTheme, "meta.source.js html");

      expect.string(style.foreground).toEqual("smoke");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);

      let style: ResolvedStyle.t = TokenTheme.match(simpleTokenTheme, "html");

      expect.string(style.foreground).toEqual("slateGray");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("parent rule gets ignored", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(simpleTokenTheme, "meta foo");

      expect.string(style.foreground).toEqual("lavender");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("foo & bar gets correctly style (compound rule)", ({expect, _}) => {
      let style: ResolvedStyle.t = TokenTheme.match(simpleTokenTheme, "foo");
      expect.bool(style.foreground == "lavender").toBe(true);
      expect.bool(style.background == "#000").toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);

      let style: ResolvedStyle.t = TokenTheme.match(simpleTokenTheme, "bar");
      expect.bool(style.foreground == "lavender").toBe(true);
      expect.bool(style.background == "#000").toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test(
      "entity.other.attribute-name.foo & bar gets correctly style (more interesting compound rule)",
      ({expect, _}) => {
        let style: ResolvedStyle.t =
          TokenTheme.match(
            simpleTokenTheme,
            "entity.other.attribute-name.foo",
          );
        expect.string(style.foreground).toEqual("salmon");
        expect.string(style.background).toEqual("#000");

        expect.bool(style.bold).toBe(true);
        expect.bool(style.italic).toBe(false);

        let style: ResolvedStyle.t =
          TokenTheme.match(
            simpleTokenTheme,
            "entity.other.attribute-name.bar",
          );
        expect.string(style.foreground).toEqual("salmon");
        expect.string(style.background).toEqual("#000");
        expect.bool(style.bold).toBe(true);
        expect.bool(style.italic).toBe(false);
      },
    );

    test("baz gets default style (no match)", ({expect, _}) => {
      let style: ResolvedStyle.t = TokenTheme.match(simpleTokenTheme, "baz");
      expect.string(style.foreground).toEqual("#fff");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("var gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t = TokenTheme.match(simpleTokenTheme, "var");
      expect.string(style.foreground).toEqual("#92D3CA");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("var.baz gets correct style (should match var)", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(simpleTokenTheme, "var.baz");
      expect.string(style.foreground).toEqual("#92D3CA");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("var.identifier gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(simpleTokenTheme, "var.identifier");
      expect.string(style.foreground).toEqual("#007fff");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(true);
      expect.bool(style.italic).toBe(false);
    });
    test("constant gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(simpleTokenTheme, "constant");

      expect.string(style.foreground).toEqual("#00FFFF");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });

    test("constant.numeric gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(simpleTokenTheme, "constant.numeric");
      expect.string(style.foreground).toEqual("#990000");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });

    test("constant.numeric.hex gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TokenTheme.match(simpleTokenTheme, "constant.numeric.hex");
      expect.string(style.foreground).toEqual("#990000");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(true);
      expect.bool(style.italic).toBe(true);
    });
  });
  describe("of_yojson", ({test, _}) => {
    test("empty array parses", ({expect, _}) => {
      let json = Yojson.Safe.from_string("[]");
      let _ =
        TokenTheme.of_yojson(
          ~defaultForeground="#fff",
          ~defaultBackground="#000",
          json,
        );
      expect.bool(true).toBe(true);
    });
    test("resolves to default colors if no match", ({expect, _}) => {
      let json = Yojson.Safe.from_string("[]");
      let theme =
        TokenTheme.of_yojson(
          ~defaultForeground="#fff",
          ~defaultBackground="#000",
          json,
        );

      let style: ResolvedStyle.t =
        TokenTheme.match(theme, "constant.numeric.hex");
      expect.string(style.foreground).toEqual("#fff");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("simple match", ({expect, _}) => {
      let json =
        Yojson.Safe.from_string(
          {|
       [{
        "name": "Text",
        "scope": "constant.numeric.hex",
        "settings": {
            "foreground": "#bbbbbb",
            "italic": true
        }
       },
       {
        "name": "Oct text",
        "scope": "constant.numeric.oct",
        "settings": {
            "foreground": "#0f0",
            "background": "#f00",
            "bold": true
        }
       }]
      |},
        );

      let theme =
        TokenTheme.of_yojson(
          ~defaultForeground="#fff",
          ~defaultBackground="#000",
          json,
        );

      let style: ResolvedStyle.t =
        TokenTheme.match(theme, "constant.numeric.hex");
      expect.string(style.foreground).toEqual("#bbbbbb");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);

      let style: ResolvedStyle.t =
        TokenTheme.match(theme, "constant.numeric.oct");
      expect.string(style.foreground).toEqual("#0f0");
      expect.string(style.background).toEqual("#f00");
      expect.bool(style.bold).toBe(true);
      expect.bool(style.italic).toBe(false);
    });
    test("compound selector", ({expect, _}) => {
      let json =
        Yojson.Safe.from_string(
          {|
       [{
        "name": "Text",
        "scope": "constant.numeric.hex, constant.numeric.oct",
        "settings": {
            "foreground": "#bbbbbb",
            "italic": true
        }
       }]
      |},
        );

      let theme =
        TokenTheme.of_yojson(
          ~defaultForeground="#fff",
          ~defaultBackground="#000",
          json,
        );

      let style: ResolvedStyle.t =
        TokenTheme.match(theme, "constant.numeric.hex");
      expect.string(style.foreground).toEqual("#bbbbbb");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);

      let style: ResolvedStyle.t =
        TokenTheme.match(theme, "constant.numeric.oct");
      expect.string(style.foreground).toEqual("#bbbbbb");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });
    test("compound array selector", ({expect, _}) => {
      let json =
        Yojson.Safe.from_string(
          {|
       [{
        "name": "Text",
        "scope": ["constant.numeric.hex", "constant.numeric.oct"],
        "settings": {
            "foreground": "#bbbbbb",
            "italic": true
        }
       }]
      |},
        );

      let theme =
        TokenTheme.of_yojson(
          ~defaultForeground="#fff",
          ~defaultBackground="#000",
          json,
        );

      let style: ResolvedStyle.t =
        TokenTheme.match(theme, "constant.numeric.hex");
      expect.string(style.foreground).toEqual("#bbbbbb");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);

      let style: ResolvedStyle.t =
        TokenTheme.match(theme, "constant.numeric.oct");
      expect.string(style.foreground).toEqual("#bbbbbb");
      expect.string(style.background).toEqual("#000");
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });
  });
});
