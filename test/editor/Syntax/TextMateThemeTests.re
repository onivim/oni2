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
      ("var", TokenStyle.create(~foreground=Colors.aqua, ())),
      ("var.identifier", TokenStyle.create(~foreground=Colors.azure, ~bold=true, ())),
      ("constant", TokenStyle.create(~foreground=Colors.cyan, ~italic=true, ())),
      ("constant.numeric", TokenStyle.create(~foreground=Colors.crimson, ())),
      ("constant.numeric.hex", TokenStyle.create(~bold=true, ())),
      ("foo, bar", TokenStyle.create(~foreground=Colors.lavender, ())),
      ("entity", TokenStyle.create(~bold=true, ())),
      
      (
        "entity.other.attribute-name.foo,entity.other.attribute-name.bar",
        TokenStyle.create(~foreground=Colors.salmon, ()),
      ),
      ("html", TokenStyle.create(~foreground=Colors.slateGray, ())),
      ("meta html", TokenStyle.create(~foreground=Colors.whiteSmoke, ())),
      ("source.php string", TokenStyle.create(~foreground=Colors.peachPuff, ())),
      ("text.html source.php", TokenStyle.create(~foreground=Colors.navy, ())),
    ]);

  describe("match", ({test, _}) => {
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
    })
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
});
