/* open Oni_Core; */
open TestFramework;

/*open Oni_Core.Types;*/
module Theme = Oni_Syntax.Theme;
module Scope = Oni_Syntax.TextMateScopes.Scope;
module Selector = Oni_Syntax.TextMateScopes.Selector;
module ResolvedStyle = Oni_Syntax.TextMateScopes.ResolvedStyle;
module TokenStyle = Oni_Syntax.TextMateScopes.TokenStyle;

describe("Theme", ({describe, _}) => {
  /* Test theme inspired by:
        https://code.visualstudio.com/blogs/2017/02/08/syntax-highlighting-optimizations#_finally-whats-new-in-vs-code-19
     */
  let simpleTheme =
    Theme.create(
      ~selectors=[
        Selector.create(
          ~style=TokenStyle.create(~foreground=9, ()),
          ~scopes=[Scope.ofString("var")],
          (),
        ),
        Selector.create(
          ~style=TokenStyle.create(~foreground=2, ~bold=true, ()),
          ~scopes=[Scope.ofString("var.identifier")],
          (),
        ),
        Selector.create(
          ~style=TokenStyle.create(~foreground=4, ~italic=true, ()),
          ~scopes=[Scope.ofString("constant")],
          (),
        ),
        Selector.create(
          ~style=TokenStyle.create(~foreground=5, ()),
          ~scopes=[Scope.ofString("constant.numeric")],
          (),
        ),
        Selector.create(
          ~style=TokenStyle.create(~bold=true, ()),
          ~scopes=[Scope.ofString("constant.numeric.hex")],
          (),
        ),
      ],
      (),
    );

  describe("match", ({test, _}) => {
    test("baz gets default style (no match)", ({expect, _}) => {
      let style: ResolvedStyle.t =
        Theme.match(simpleTheme, [Scope.ofString("baz")]);
      expect.int(style.foreground).toBe(1);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    test("var gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        Theme.match(simpleTheme, [Scope.ofString("var")]);
      expect.int(style.foreground).toBe(9);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });
    
    test("var.baz gets correct style (should match var)", ({expect, _}) => {
      let style: ResolvedStyle.t =
        Theme.match(simpleTheme, [Scope.ofString("var.baz")]);
      expect.int(style.foreground).toBe(9);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(false);
    });

    test("var.identifier gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        Theme.match(simpleTheme, [Scope.ofString("var.identifier")]);
      expect.int(style.foreground).toBe(2);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(true);
      expect.bool(style.italic).toBe(false);
    });
    
    test("constant gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        Theme.match(simpleTheme, [Scope.ofString("constant")]);
      expect.int(style.foreground).toBe(4);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });
    
    test("constant.numeric gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        Theme.match(simpleTheme, [Scope.ofString("constant.numeric")]);
      expect.int(style.foreground).toBe(5);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(false);
      expect.bool(style.italic).toBe(true);
    });
    
    test("constant.numeric.hex gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        Theme.match(simpleTheme, [Scope.ofString("constant.numeric.hex")]);
      expect.int(style.foreground).toBe(5);
      expect.int(style.background).toBe(0);
      expect.bool(style.bold).toBe(true);
      expect.bool(style.italic).toBe(true);
    });
  });
});
