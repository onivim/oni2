open TestFramework;

module Pattern = Textmate.Pattern;

describe("Pattern", ({describe, _}) => {
  describe("json parsing", ({test, _}) => {
    test("include", ({expect, _}) => {
      let inc = Pattern.Json.of_string("source", {|{ "include": "#value" }|});
      expect.bool(inc == Ok(Pattern.Include("source", "#value"))).toBe(
        true,
      );
      let inc2 =
        Pattern.Json.of_string("source", {|{ "include": "#value2" }|});
      expect.bool(inc2 == Ok(Pattern.Include("source", "#value2"))).toBe(
        true,
      );
    });
    test("match", ({expect, _}) => {
      let match1 =
        Pattern.Json.of_string(
          "source",
          {|{ "match": "a|b|c", name: "match1" }|},
        );

      switch (match1) {
      | Ok(Match(v)) =>
        expect.bool(v.matchName == Some("match1")).toBe(true);
        expect.int(List.length(v.captures)).toBe(0);
      | _ => failwith("Parse failed for match")
      };

      let matchWithCapture =
        Pattern.Json.of_string(
          "source",
          {|{ "match": "a|b|c", name: "match2", captures: { "0": { "name": "derp" } } }|},
        );

      switch (matchWithCapture) {
      | Ok(Match(v)) =>
        expect.bool(v.matchName == Some("match2")).toBe(true);
        expect.int(List.length(v.captures)).toBe(1);
      | _ => failwith("Parse failed for match")
      };
    });
    test("matchRange", ({expect, _}) => {
      let matchRange1 =
        Pattern.Json.of_string(
          "source",
          {|
          {
              "begin": "\\[",
              "beginCaptures": { "0": { "name": "array-json-begin" } },
              "end": "\\]",
              "name": "array-json",
              "endCaptures": { "0": { "name": "array-json-end" } },
              "patterns": [
                  { "include": "#value" }
              ]
          }
        |},
        );

      switch (matchRange1) {
      | Ok(MatchRange(v)) =>
        expect.bool(v.name == Some("array-json")).toBe(true);
        expect.int(List.length(v.beginCaptures)).toBe(1);
        expect.int(List.length(v.endCaptures)).toBe(1);
      | _ => failwith("Parse failed for match")
      };
    });
  })
});
