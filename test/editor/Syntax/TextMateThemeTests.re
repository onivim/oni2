open TestFramework;

module TextMateTheme = Oni_Syntax.TextMateTheme;
module Scope = Oni_Syntax.TextMateScopes.Scope;
module Selector = Oni_Syntax.TextMateScopes.Selector;
module ResolvedStyle = Oni_Syntax.TextMateScopes.ResolvedStyle;
module TokenStyle = Oni_Syntax.TextMateScopes.TokenStyle;

open Revery;

describe("TextMateTheme", ({describe, _}) => {
  /* Test theme inspired by:
        https://code.visualstudio.com/blogs/2017/02/08/syntax-highlighting-optimizations#_finally-whats-new-in-vs-code-19
     */

  let simpleTextMateTheme =
    TextMateTheme.create(
      ~defaultBackground=Colors.black,
      ~defaultForeground=Colors.white,
      [
        ("var", TokenStyle.create(~foreground=Some(Colors.aqua), ())),
        (
          "var.identifier",
          TokenStyle.create(
            ~foreground=Some(Colors.azure),
            ~bold=Some(true),
            (),
          ),
        ),
        (
          "constant",
          TokenStyle.create(
            ~foreground=Some(Colors.cyan),
            ~italic=Some(true),
            (),
          ),
        ),
        (
          "constant.numeric",
          TokenStyle.create(~foreground=Some(Colors.crimson), ()),
        ),
        ("constant.numeric.hex", TokenStyle.create(~bold=Some(true), ())),
        (
          "foo, bar",
          TokenStyle.create(~foreground=Some(Colors.lavender), ()),
        ),
        ("entity", TokenStyle.create(~bold=Some(true), ())),
        (
          "entity.other.attribute-name.foo,entity.other.attribute-name.bar",
          TokenStyle.create(~foreground=Some(Colors.salmon), ()),
        ),
        ("html", TokenStyle.create(~foreground=Some(Colors.slateGray), ())),
        (
          "meta html",
          TokenStyle.create(~foreground=Some(Colors.whiteSmoke), ()),
        ),
        (
          "source.php string",
          TokenStyle.create(~foreground=Some(Colors.peachPuff), ()),
        ),
        (
          "text.html source.php",
          TokenStyle.create(~foreground=Some(Colors.navy), ()),
        ),
        (
          "text.html source.js",
          TokenStyle.create(
            ~foreground=Some(Colors.navy),
            ~background=Some(Colors.cornflowerBlue),
            (),
          ),
        ),
      ],
    );

  describe("match", ({test, _}) => {
    test("background color should be picked up", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "text.html.basic source.js");

      expect.bool(style.foreground == Colors.navy).toBe(true);
      expect.bool(style.background == Colors.cornflowerBlue).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test(
      "deeper rule should win (source.php string over text.html source.php)",
      ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(
          simpleTextMateTheme,
          "text.html.basic source.php.html string.quoted",
        );

      expect.bool(style.foreground == Colors.peachPuff).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("parent rule (meta html) gets applied", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "meta html");

      expect.bool(style.foreground == Colors.whiteSmoke).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);

      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "meta.source.js html");

      expect.bool(style.foreground == Colors.whiteSmoke).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);

      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "html");

      expect.bool(style.foreground == Colors.slateGray).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("parent rule gets ignored", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "meta foo");

      expect.bool(style.foreground == Colors.lavender).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("foo & bar gets correctly style (compound rule)", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "foo");
      expect.bool(style.foreground == Colors.lavender).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);

      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "bar");
      expect.bool(style.foreground == Colors.lavender).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test(
      "entity.other.attribute-name.foo & bar gets correctly style (more interesting compound rule)",
      ({expect, _}) => {
        let style: ResolvedStyle.t =
          TextMateTheme.match(
            simpleTextMateTheme,
            "entity.other.attribute-name.foo",
          );
        expect.bool(style.foreground == Colors.salmon).toBe(true);
        expect.bool(style.background == Colors.black).toBe(true);
        expect.bool(style.bold).toBe(true);
        expect.bool(style.italic).toBe(false);

        let style: ResolvedStyle.t =
          TextMateTheme.match(
            simpleTextMateTheme,
            "entity.other.attribute-name.bar",
          );
        expect.bool(style.foreground == Colors.salmon).toBe(true);
        expect.bool(style.background == Colors.black).toBe(true);
        expect.bool(style.bold).toBe(true);
        expect.bool(style.italic).toBe(false);
      },
    );

    test("baz gets default style (no match)", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "baz");
      expect.bool(style.foreground == Colors.white).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("var gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "var");
      expect.bool(style.foreground == Colors.aqua).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("var.baz gets correct style (should match var)", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "var.baz");
      expect.bool(style.foreground == Colors.aqua).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("var.identifier gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "var.identifier");
      expect.bool(style.foreground == Colors.azure).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(true);
      expect.bool(style.italic).toBe(false);
    });
    test("constant gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "constant");

      expect.bool(style.foreground == Colors.cyan).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });

    test("constant.numeric gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "constant.numeric");
      expect.bool(style.foreground == Colors.crimson).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });

    test("constant.numeric.hex gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "constant.numeric.hex");
      expect.bool(style.foreground == Colors.crimson).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(true);
      expect.bool(style.italic).toBe(true);
    });
  });
  describe("of_yojson", ({test, _}) => {
    test("empty array parses", ({expect, _}) => {
      let json = Yojson.Safe.from_string("[]");
      let _ =
        TextMateTheme.of_yojson(
          ~defaultForeground=Colors.white,
          ~defaultBackground=Colors.black,
          json,
        );
      expect.bool(true).toBe(true);
    });
    test("resolves to default colors if no match", ({expect, _}) => {
      let json = Yojson.Safe.from_string("[]");
      let theme =
        TextMateTheme.of_yojson(
          ~defaultForeground=Colors.white,
          ~defaultBackground=Colors.black,
          json,
        );

      let style: ResolvedStyle.t =
        TextMateTheme.match(theme, "constant.numeric.hex");
      expect.bool(style.foreground == Colors.white).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
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
        TextMateTheme.of_yojson(
          ~defaultForeground=Colors.white,
          ~defaultBackground=Colors.black,
          json,
        );

      let style: ResolvedStyle.t =
        TextMateTheme.match(theme, "constant.numeric.hex");
      expect.bool(style.foreground == Color.hex("#bbbbbb")).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);

      let style: ResolvedStyle.t =
        TextMateTheme.match(theme, "constant.numeric.oct");
      expect.bool(style.foreground == Color.hex("#0f0")).toBe(true);
      expect.bool(style.background == Color.hex("#f00")).toBe(true);
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
        TextMateTheme.of_yojson(
          ~defaultForeground=Colors.white,
          ~defaultBackground=Colors.black,
          json,
        );

      let style: ResolvedStyle.t =
        TextMateTheme.match(theme, "constant.numeric.hex");
      expect.bool(style.foreground == Color.hex("#bbbbbb")).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);

      let style: ResolvedStyle.t =
        TextMateTheme.match(theme, "constant.numeric.oct");
      expect.bool(style.foreground == Color.hex("#bbbbbb")).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
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
        TextMateTheme.of_yojson(
          ~defaultForeground=Colors.white,
          ~defaultBackground=Colors.black,
          json,
        );

      let style: ResolvedStyle.t =
        TextMateTheme.match(theme, "constant.numeric.hex");
      expect.bool(style.foreground == Color.hex("#bbbbbb")).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);

      let style: ResolvedStyle.t =
        TextMateTheme.match(theme, "constant.numeric.oct");
      expect.bool(style.foreground == Color.hex("#bbbbbb")).toBe(true);
      expect.bool(style.background == Colors.black).toBe(true);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });
  });
});
