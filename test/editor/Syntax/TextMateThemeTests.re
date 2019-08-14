open TestFramework;

module TextMateTheme = Oni_Syntax.TextMateTheme;
module Scope = Oni_Syntax.TextMateScopes.Scope;
module Selector = Oni_Syntax.TextMateScopes.Selector;
module ResolvedStyle = Oni_Syntax.TextMateScopes.ResolvedStyle;
module TokenStyle = Oni_Syntax.TextMateScopes.TokenStyle;

describe("TextMateTheme", ({describe, _}) => {
  /* Test theme inspired by:
        https://code.visualstudio.com/blogs/2017/02/08/syntax-highlighting-optimizations#_finally-whats-new-in-vs-code-19
     */
  let simpleTextMateTheme =
    TextMateTheme.create([
      ("var", TokenStyle.create(~foreground=9, ())),
      ("var.identifier", TokenStyle.create(~foreground=2, ~bold=true, ())),
      ("constant", TokenStyle.create(~foreground=4, ~italic=true, ())),
      ("constant.numeric", TokenStyle.create(~foreground=5, ())),
      ("constant.numeric.hex", TokenStyle.create(~bold=true, ())),
      ("foo, bar", TokenStyle.create(~foreground=10, ())),
      ("entity", TokenStyle.create(~bold=true, ())),
      (
        "entity.other.attribute-name.foo,entity.other.attribute-name.bar",
        TokenStyle.create(~foreground=11, ()),
      ),
      ("html", TokenStyle.create(~foreground=12, ())),
      ("meta html", TokenStyle.create(~foreground=14, ())),
      ("source.php string", TokenStyle.create(~foreground=15, ())),
      ("text.html source.php", TokenStyle.create(~foreground=16, ())),
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

      expect.int(style.foreground).toBe(15);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("parent rule (meta html) gets applied", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "meta html");

      expect.int(style.foreground).toBe(14);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);

      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "meta.source.js html");

      expect.int(style.foreground).toBe(14);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);

      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "html");

      expect.int(style.foreground).toBe(12);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("parent rule gets ignored", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "meta foo");

      expect.int(style.foreground).toBe(10);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("foo & bar gets correctly style (compound rule)", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "foo");
      expect.int(style.foreground).toBe(10);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);

      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "bar");
      expect.int(style.foreground).toBe(10);
      expect.int(style.background).toBe(0);
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
        expect.int(style.foreground).toBe(11);
        expect.int(style.background).toBe(0);
        expect.bool(style.bold).toBe(true);
        expect.bool(style.italic).toBe(false);

        let style: ResolvedStyle.t =
          TextMateTheme.match(
            simpleTextMateTheme,
            "entity.other.attribute-name.bar",
          );
        expect.int(style.foreground).toBe(11);
        expect.int(style.background).toBe(0);
        expect.bool(style.bold).toBe(true);
        expect.bool(style.italic).toBe(false);
      },
    );
    test("baz gets default style (no match)", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "baz");
      expect.int(style.foreground).toBe(1);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("var gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "var");
      expect.int(style.foreground).toBe(9);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });

    test("var.baz gets correct style (should match var)", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "var.baz");
      expect.int(style.foreground).toBe(9);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });

    test("var.identifier gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "var.identifier");
      expect.int(style.foreground).toBe(2);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(true);
      expect.bool(style.italic).toBe(false);
    });

    test("constant gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "constant");
      expect.int(style.foreground).toBe(4);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });

    test("constant.numeric gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "constant.numeric");
      expect.int(style.foreground).toBe(5);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });

    test("constant.numeric.hex gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        TextMateTheme.match(simpleTextMateTheme, "constant.numeric.hex");
      expect.int(style.foreground).toBe(5);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(true);
      expect.bool(style.italic).toBe(true);
    });
  });
});
