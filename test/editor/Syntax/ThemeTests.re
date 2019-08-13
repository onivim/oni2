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
        https://github.com/revery-ui/reason-font-manager
     */
  let simpleTheme =
    Theme.create(
      ~selectors=[
        Selector.create(
          ~style=TokenStyle.create(~foreground=1, ()),
          ~scopes=[Scope.ofString("var")],
          (),
        ),
        Selector.create(
          ~style=TokenStyle.create(~foreground=2, ~bold=true, ()),
          ~scopes=[Scope.ofString("var.identifier")],
          (),
        ),
      ],
      (),
    );

  describe("match", ({test, _}) => {
    test("var gets correct style", ({expect, _}) => {
      let style: ResolvedStyle.t =
        Theme.match(simpleTheme, [Scope.ofString("var")]);
      expect.int(style.foreground).toBe(1);
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
  });
});
