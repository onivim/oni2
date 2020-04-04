open TestFramework;

let re = (pattern, str) => {
  let re = pattern |> Re.Pcre.re |> Re.compile;
  Re.execp(re, str);
};

describe("WhenExpr", ({describe, _}) => {
  describe("vscode tests", ({describe, _}) => {
    // Translkated from https://github.com/microsoft/vscode/blob/7fc5d9150569247b3494eb3715a078bf7f8e9272/src/vs/platform/contextkey/test/common/contextkey.test.ts
    // TODO: equals
    // TODO: normalize
    describe("evaluate", ({test, _}) => {
      let context = Hashtbl.create(4);
      Hashtbl.add(context, "a", WhenExpr.Value.True);
      Hashtbl.add(context, "b", WhenExpr.Value.False);
      Hashtbl.add(context, "c", WhenExpr.Value.String("5"));
      Hashtbl.add(context, "d", WhenExpr.Value.String("d"));

      let getValue = name =>
        switch (Hashtbl.find_opt(context, name)) {
        | Some(value) => value
        | None => WhenExpr.Value.False
        };

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
  })
});
