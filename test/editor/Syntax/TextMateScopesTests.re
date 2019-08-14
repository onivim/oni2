/* open Oni_Core; */
open TestFramework;

/*open Oni_Core.Types;*/
module TextMateScopes = Oni_Syntax.TextMateScopes;
module Scope = TextMateScopes.Scope;
module Scopes = TextMateScopes.Scopes;

describe("TextMateScopes", ({describe, _}) => {
  describe("Scope", ({test, _}) =>
    test("matches", ({expect, _}) => {
      let testScope = Scope.ofString("string.quoted.double.php");

      // True cases
      expect.bool(Scope.matches([], testScope)).toBe(true);
      expect.bool(Scope.matches(["string"], testScope)).toBe(true);
      expect.bool(Scope.matches(["string", "quoted"], testScope)).toBe(
        true,
      );
      expect.bool(
        Scope.matches(["string", "quoted", "double", "php"], testScope),
      ).
        toBe(
        true,
      );

      // False cases
      expect.bool(
        Scope.matches(["string", "notquoted", "double", "php"], testScope),
      ).
        toBe(
        false,
      );
      expect.bool(Scope.matches(["var"], testScope)).toBe(false);
    })
  );

  describe("Scopes", ({test, _}) =>
    test("matches", ({expect, _}) => {
      let testScopes =
        Scopes.ofString(
          "text.html.basic source.php.embedded.html string.quoted.double.php",
        );

      // True cases
      expect.bool(Scopes.matches([], testScopes)).toBe(true);
      expect.bool(Scopes.matches(Scopes.ofString("text.html"), testScopes)).
        toBe(
        true,
      );
      expect.bool(
        Scopes.matches(Scopes.ofString("text.html string"), testScopes),
      ).
        toBe(
        true,
      );
      expect.bool(
        Scopes.matches(
          Scopes.ofString("text.html source.php string"),
          testScopes,
        ),
      ).
        toBe(
        true,
      );

      // False cases
      expect.bool(
        Scopes.matches(Scopes.ofString("text.html var"), testScopes),
      ).
        toBe(
        false,
      );
      expect.bool(
        Scopes.matches(
          Scopes.ofString("text.html source.php var"),
          testScopes,
        ),
      ).
        toBe(
        false,
      );
      expect.bool(Scopes.matches(Scopes.ofString("var"), testScopes)).toBe(
        false,
      );
    })
  );
});
