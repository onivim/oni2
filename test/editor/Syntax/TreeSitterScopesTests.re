open TestFramework;

module TreeSitterScopes = Oni_Syntax.TreeSitterScopes;

open TreeSitterScopes;

describe("TreeSitterScopes", ({describe, _}) => {
  describe("TextMateConverter", ({test, describe, _}) => {
    // Create a simple converter... this is a representation of grammars
    // like: https://github.com/atom/language-json/blob/04f1fbd5eb3aabcfc91b30a2c091a9fc657438ee/grammars/tree-sitter-json.cson#L48
    let simpleConverter =
      TextMateConverter.create([
        ("value", [Scope("source.json")]),
        ("object", [Scope("meta.structure.dictionary.json")]),
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

    test("returns None for non-existent scopes", ({expect, _}) => {
      let nonExistentScope =
        TextMateConverter.getTextMateScope(
          ~path=["non-existent-tree-sitter-scope"],
          simpleConverter,
        );

      expect.bool(nonExistentScope == None).toBe(true);
    });
    test("matches simple scopes", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(~path=["value"], simpleConverter);

      expect.bool(scope == Some("source.json")).toBe(true);
    });
    test("matches another simple scope", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["object"],
          simpleConverter,
        );

      expect.bool(scope == Some("meta.structure.dictionary.json")).toBe(
        true,
      );
    });
    test("regex match failure", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string_content"],
          ~token="derp",
          simpleConverter,
        );

      expect.bool(scope == None).toBe(true);
    });
    test("regex match success", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string_content"],
          ~token="https://v2.onivim.io",
          simpleConverter,
        );

      expect.bool(scope == Some("markup.underline.link.https.hyperlink")).
        toBe(
        true,
      );
    });
    test("child selector", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string", "pair"],
          simpleConverter,
        );

      expect.bool(scope == Some("constant.language")).toBe(true);
    });
    test("child selector only matches direct children", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~path=["string", "random-scope", "pair"],
          simpleConverter,
        );

      expect.bool(scope == None).toBe(true);
    });
    test("child selector matches descendants", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~index=0,
          ~path=["string", "pair", "random-scope"],
          simpleConverter,
        );

      expect.bool(scope == Some("constant.language")).toBe(true);
    });
    test("nth-child(1) selector matches", ({expect, _}) => {
      let scope =
        TextMateConverter.getTextMateScope(
          ~index=1,
          ~path=["string", "pair", "random-scope"],
          simpleConverter,
        );

      expect.bool(scope == Some("string.quoted.dictionary.key.json")).toBe(
        true,
      );
    });

    describe("of_yojson", ({test, _}) => {
      test("empty json dictionary", ({expect, _}) => {
        let json = Yojson.Safe.from_string({| { } |});

        let converter = TextMateConverter.of_yojson(json);
        expect.bool(converter == TextMateConverter.empty).toBe(true);
      });
      test("single item dictionary", ({expect, _}) => {
        let json = Yojson.Safe.from_string({| { 
        "value": "source.json"
        } |});

        let converter = TextMateConverter.of_yojson(json);

        let scope = TextMateConverter.getTextMateScope(
          ~index=1,
          ~path=["value"],
          converter,
        );

        expect.bool(scope == Some("source.json")).toBe(true);
      });
      test("multiple item dictionary", ({expect, _}) => {
        let json = Yojson.Safe.from_string({| { 
        "value": "source.json",
        "object": "meta.structure.dictionary.json"
        } |});

        let converter = TextMateConverter.of_yojson(json);

        let scope1 = TextMateConverter.getTextMateScope(
          ~index=1,
          ~path=["value"],
          converter,
        );
        let scope2 = TextMateConverter.getTextMateScope(
          ~index=1,
          ~path=["object"],
          converter,
        );

        expect.bool(scope1 == Some("source.json")).toBe(true);
        expect.bool(scope2 == Some("meta.structure.dictionary.json")).toBe(true);
      });
      test("list of matchers", ({expect, _}) => {
        let json = Yojson.Safe.from_string({| { 
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
        } |});

        let converter = TextMateConverter.of_yojson(json);

        let scopeNoMatch = TextMateConverter.getTextMateScope(
          ~index=1,
          ~path=["string_content"],
          converter,
        );
        let scopeHttpMatch = TextMateConverter.getTextMateScope(
          ~index=1,
          ~path=["string_content"],
          ~token="http://v2.onivim.io",
          converter,
        );
        let scopeHttpsMatch = TextMateConverter.getTextMateScope(
          ~index=1,
          ~path=["string_content"],
          ~token="https://v2.onivim.io",
          converter,
        );

        expect.bool(scopeNoMatch == None).toBe(true);
        expect.bool(scopeHttpMatch == Some("markup.underline.link.http.hyperlink")).toBe(true);
        expect.bool(scopeHttpsMatch == Some("markup.underline.link.https.hyperlink")).toBe(true);
      });
    });
  });
});
