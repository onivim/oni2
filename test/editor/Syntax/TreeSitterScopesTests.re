open TestFramework;

module TreeSitterScopes = Oni_Syntax.TreeSitterScopes;

open TreeSitterScopes;

describe("TreeSitterScopes", ({describe, _}) =>
  describe("TextMateConverter", ({test, _ /*describe,*/}) => {
    // Create a simple converter... this is a representation of grammars
    // like: https://github.com/atom/language-json/blob/04f1fbd5eb3aabcfc91b30a2c091a9fc657438ee/grammars/tree-sitter-json.cson#L48
    let simpleConverter =
      TextMateConverter.create([
        ("value", [Scope("source.json")]),
        ("object", [Scope("meta.structure.dictionary.json")]),
        ("string", [Scope("string.quoted.double")]),
        (
          "string_content",
          [
            RegExMatch(
              Str.regexp("^http:\\/\\/"),
              "markup.underline.link.http.hyperlink",
            ),
            RegExMatch(
              Str.regexp("^https:\\/\\/"),
              "markup.underline.link.https.hyperlink",
            ),
          ],
        ),
        ("pair > string", [Scope("constant.language")]),
        (
          "pair > string:nth-child(1)",
          [Scope("string.quoted.dictionary.key.json")],
        ),
      ]);

    test("simple hierarchy", ({expect, _}) => {
      let scope1 =
        TextMateConverter.getTextMateScope(
          ~path=["string", "array"],
          simpleConverter,
        );

      expect.string(scope1).toEqual("string.quoted.double");

      let scope2 =
        TextMateConverter.getTextMateScope(
          ~path=["string", "object"],
          simpleConverter,
        );

      expect.string(scope2).toEqual(
        "meta.structure.dictionary.json string.quoted.double",
      );
    });
    test("non-matching child rule doesn't break other rules", ({expect, _}) => {
      let stringConverter =
        TextMateConverter.create([
          ("string", [Scope("string.quoted.double")]),
          (
            "pair > string:nth-child(0)",
            [Scope("string.quoted.dictionary.key.json")],
          ),
        ]);

      let scope1 =
        TextMateConverter.getTextMateScope(
          ~path=["string"],
          stringConverter,
        );

      expect.string(scope1).toEqual("string.quoted.double");

      let scope2 =
        TextMateConverter.getTextMateScope(
          ~path=["string", "array"],
          stringConverter,
        );

      expect.string(scope2).toEqual("string.quoted.double");
    });
    test("returns empty for non-existent scopes", ({expect, _}) => {
      let nonExistentScope =
        TextMateConverter.getTextMateScope(
          ~path=["non-existent-tree-sitter-scope"],
          simpleConverter,
        );

      expect.bool(nonExistentScope == "").toBe(true);
    });
    test("matches simple scopes", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(~path=["value"], simpleConverter);

      expect.string(scope).toEqual("source.json");
    });
    test("matches another simple scope", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["object"],
          simpleConverter,
        );

      expect.string(scope).toEqual("meta.structure.dictionary.json");
    });
    test("regex match failure", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string_content"],
          ~token="derp",
          simpleConverter,
        );

      expect.string(scope).toEqual("");
    });
    test("regex match success", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string_content"],
          ~token="https://v2.onivim.io",
          simpleConverter,
        );

      expect.string(scope).toEqual("markup.underline.link.https.hyperlink");
    });
    test("child selector", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string", "pair"],
          simpleConverter,
        );

      expect.string(scope).toEqual("constant.language");
    });
    test("child selector only matches direct children", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string", "random-scope", "pair"],
          simpleConverter,
        );

      expect.string(scope).toEqual("string.quoted.double");
    });
    test("child selector matches descendants", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~index=0,
          ~path=["string", "pair", "random-scope"],
          simpleConverter,
        );

      expect.string(scope).toEqual("constant.language");
    });
    test("nth-child(1) selector matches", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~index=1,
          ~path=["string", "pair", "random-scope"],
          simpleConverter,
        );

      expect.string(scope).toEqual("string.quoted.dictionary.key.json");
    });

    describe("of_yojson", ({test, _}) => {
      test("empty json dictionary", ({expect, _}) => {
        let json = Yojson.Safe.from_string({| { } |});

        let converter = TextMateConverter.of_yojson(json);
        expect.bool(converter == TextMateConverter.empty).toBe(true);
      });
      test("single item dictionary", ({expect, _}) => {
        let json =
          Yojson.Safe.from_string(
            {| {
        "value": "source.json"
        } |},
          );

        let converter = TextMateConverter.of_yojson(json);

        let scope =
          TextMateConverter.getTextMateScope(
            ~index=1,
            ~path=["value"],
            converter,
          );

        expect.string(scope).toEqual("source.json");
      });
      test("multiple item dictionary", ({expect, _}) => {
        let json =
          Yojson.Safe.from_string(
            {| {
        "value": "source.json",
        "object": "meta.structure.dictionary.json"
        } |},
          );

        let converter = TextMateConverter.of_yojson(json);

        let scope1 =
          TextMateConverter.getTextMateScope(
            ~index=1,
            ~path=["value"],
            converter,
          );
        let scope2 =
          TextMateConverter.getTextMateScope(
            ~index=1,
            ~path=["object"],
            converter,
          );

        expect.string(scope1).toEqual("source.json");
        expect.string(scope2).toEqual("meta.structure.dictionary.json");
      });
      test("list of matchers", ({expect, _}) => {
        let json =
          Yojson.Safe.from_string(
            {| {
        "string_content": [
          {
            "match": "^http://",
            "scopes": "markup.underline.link.http.hyperlink"
          },
          {
            "match": "^https://",
            "scopes": "markup.underline.link.https.hyperlink"
          }
        ]
        } |},
          );

        let converter = TextMateConverter.of_yojson(json);

        let scopeNoMatch =
          TextMateConverter.getTextMateScope(
            ~index=1,
            ~path=["string_content"],
            converter,
          );
        let scopeHttpMatch =
          TextMateConverter.getTextMateScope(
            ~index=1,
            ~path=["string_content"],
            ~token="http://v2.onivim.io",
            converter,
          );
        let scopeHttpsMatch =
          TextMateConverter.getTextMateScope(
            ~index=1,
            ~path=["string_content"],
            ~token="https://v2.onivim.io",
            converter,
          );

        expect.string(scopeNoMatch).toEqual("");
        expect.string(scopeHttpMatch).toEqual(
          "markup.underline.link.http.hyperlink",
        );
        expect.string(scopeHttpsMatch).toEqual(
          "markup.underline.link.https.hyperlink",
        );
      });
      test("fallback to scope matcher", ({expect, _}) => {
        let json =
          Yojson.Safe.from_string(
            {| {
        "comment": [
          {
            "exact": "// special //",
            "scopes": "comment.special"
          },
          {
            "match": "^//",
            "scopes": "comment.line"
          },
          "comment.block"
        ]
        } |},
          );

        let converter = TextMateConverter.of_yojson(json);

        let scopeCommentLineMatch =
          TextMateConverter.getTextMateScope(
            ~index=1,
            ~path=["comment"],
            ~token="// hello",
            converter,
          );
        let scopeCommentBlockMatch =
          TextMateConverter.getTextMateScope(
            ~index=1,
            ~path=["comment"],
            ~token="test",
            converter,
          );

        let scopeCommentSpecialMatch =
          TextMateConverter.getTextMateScope(
            ~index=1,
            ~path=["comment"],
            ~token="// special //",
            converter,
          );

        expect.string(scopeCommentLineMatch).toEqual("comment.line");
        expect.string(scopeCommentSpecialMatch).toEqual("comment.special");
        expect.string(scopeCommentBlockMatch).toEqual("comment.block");
      });
    });
  })
);
