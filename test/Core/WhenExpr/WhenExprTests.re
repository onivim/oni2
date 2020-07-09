open TestFramework;

let re = (pattern, str) => {
  let re = pattern |> Re.Pcre.re |> Re.compile;
  Re.execp(re, str);
};

let createContext = pairs => {
  let context = Hashtbl.create(List.length(pairs));
  List.iter(((k, v)) => Hashtbl.add(context, k, v), pairs);

  name =>
    switch (Hashtbl.find_opt(context, name)) {
    | Some(value) => value
    | None => WhenExpr.Value.False
    };
};

describe("WhenExpr", ({describe, _}) => {
  describe("vscode tests", ({describe, _}) => {
    // Translkated from https://github.com/microsoft/vscode/blob/7fc5d9150569247b3494eb3715a078bf7f8e9272/src/vs/platform/contextkey/test/common/contextkey.test.ts
    // TODO: equals
    // TODO: normalize
    describe("evaluate", ({test, _}) => {
      let getValue =
        createContext(
          WhenExpr.Value.[
            ("a", True),
            ("b", False),
            ("c", String("5")),
            ("d", String("d")),
          ],
        );

      let testExpression = (expr, expected) =>
        test(
          expr,
          ({expect, _}) => {
            let rules = WhenExpr.parse(expr);
            // Console.log(WhenExpr.show(rules));
            expect.bool(WhenExpr.evaluate(rules, getValue)).toBe(expected);
          },
        );
      let testBatch = (expr, value) => {
        testExpression(expr, WhenExpr.Value.asBool(value));
        testExpression(expr ++ " == true", WhenExpr.Value.asBool(value));
        testExpression(expr ++ " != true", !WhenExpr.Value.asBool(value));
        testExpression(expr ++ " == false", !WhenExpr.Value.asBool(value));
        testExpression(expr ++ " != false", WhenExpr.Value.asBool(value));
        testExpression(expr ++ " == 5", value == String("5"));
        testExpression(expr ++ " != 5", value != String("5"));
        testExpression("!" ++ expr, !WhenExpr.Value.asBool(value));
        testExpression(
          expr ++ " =~ /d.*/",
          re("d.*", WhenExpr.Value.asString(value)),
        );
        testExpression(
          expr ++ " =~ /D/i",
          re("D", WhenExpr.Value.asString(value)),
        );
      };

      testBatch("a", WhenExpr.Value.True);
      testBatch("b", WhenExpr.Value.False);
      testBatch("c", WhenExpr.Value.String("5"));
      testBatch("d", WhenExpr.Value.String("d"));
      testBatch("z", WhenExpr.Value.False); // `undefined` in vscode tests, but that's nonsense

      testExpression("a && !b", true && !false);
      testExpression("a && b", true && false);
      testExpression("a && !b && c == 5", true && !false && "5" == "5");
    })
  });

  describe("regex", ({test, _}) => {
    test("1729-gitlens-regression", ({expect, _}) => {
      let getValue =
        createContext(
          WhenExpr.Value.[
            ("gitlens:activeFileStatus", String("some revision or other")),
            ("resourceScheme", String("file$")),
          ],
        );

      let expr = "gitlens:activeFileStatus =~ /revision/ && resourceScheme =~ /^(?!(file|git)$).*$/";
      let rules = WhenExpr.parse(expr);
      //Console.log(WhenExpr.show(rules));
      expect.bool(WhenExpr.evaluate(rules, getValue)).toBe(true);
    })
  });
});
