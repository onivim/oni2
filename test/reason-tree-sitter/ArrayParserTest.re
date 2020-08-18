open EditorCoreTypes;
open Treesitter;
open TestFramework;

describe("ArrayParser", ({describe, _}) => {
  describe("parse", ({test, _}) => {
    test("parses a single line array", ({expect, _}) => {
      let jsonParser = Parser.json();
      let (tree, _) = ArrayParser.parse(jsonParser, None, [|"[1, \"2\"]"|]);
      let node = Tree.getRootNode(tree);
      let ret = Node.toString(node);
      prerr_endline("RET: " ++ ret);
      expect.string(ret).toEqual(
        "(value (array (number) (string (string_content))))",
      );
    });
    test("parses a multi-line array", ({expect, _}) => {
      let multiLineArray = [|"[", "1,", "\"2\"", "]", ""|];

      let jsonParser = Parser.json();
      let (tree, _) = ArrayParser.parse(jsonParser, None, multiLineArray);
      let node = Tree.getRootNode(tree);
      let ret = Node.toString(node);
      prerr_endline("RET: " ++ ret);
      expect.string(ret).toEqual(
        "(value (array (number) (string (string_content))))",
      );
    });
  });
  describe("incremental parse", ({test, _}) => {
    test("incrementally update single line", ({expect, _}) => {
      let jsonParser = Parser.json();
      let (_, baseline) =
        ArrayParser.parse(jsonParser, None, [|"[1, \"2\"]"|]);

      let update = [|"[1]"|];
      let delta = ArrayParser.Delta.create(baseline, 0, 1, update);

      let (tree, _) = ArrayParser.parse(jsonParser, Some(delta), update);

      let node = Tree.getRootNode(tree);
      let ret = Node.toString(node);
      prerr_endline("RET: " ++ ret);
      expect.string(ret).toEqual("(value (array (number)))");
    });

    test("change single line", ({expect, _}) => {
      let start = [|"[", "1,", "\"2\",", "3", "]", ""|];

      let endv = [|"[", "1,", "2,", "3", "]", ""|];

      let jsonParser = Parser.json();
      let (_, baseline) = ArrayParser.parse(jsonParser, None, start);

      let update = [|"2,"|];
      let delta = ArrayParser.Delta.create(baseline, 2, 3, update);

      let (tree, _) = ArrayParser.parse(jsonParser, Some(delta), endv);

      let node = Tree.getRootNode(tree);
      let ret = Node.toString(node);
      prerr_endline("RET: " ++ ret);
      expect.string(ret).toEqual(
        "(value (array (number) (number) (number)))",
      );
    });

    test("remove multiple lines", ({expect, _}) => {
      let start = [|"[", "1,", "\"2\",", "3", "]", ""|];

      let endv = [|"[", "]", ""|];

      let jsonParser = Parser.json();
      let (_, baseline) = ArrayParser.parse(jsonParser, None, start);

      let update = [||];
      let delta = ArrayParser.Delta.create(baseline, 1, 4, update);

      let (tree, _) = ArrayParser.parse(jsonParser, Some(delta), endv);

      let node = Tree.getRootNode(tree);
      let ret = Node.toString(node);
      prerr_endline("RET: " ++ ret);
      expect.string(ret).toEqual("(value (array))");
    });

    test("add multiple lines", ({expect, _}) => {
      let start = [|"[", "]", ""|];

      let endv = [|"[", "1,", "\"2\",", "3", "]", ""|];

      let jsonParser = Parser.json();
      let (_, baseline) = ArrayParser.parse(jsonParser, None, start);

      let update = [|"1,", "\"2\",", "3"|];
      let delta = ArrayParser.Delta.create(baseline, 1, 1, update);

      let (tree, _) = ArrayParser.parse(jsonParser, Some(delta), endv);

      let node = Tree.getRootNode(tree);
      let ret = Node.toString(node);
      prerr_endline("RET: " ++ ret);
      expect.string(ret).toEqual(
        "(value (array (number) (string (string_content)) (number)))",
      );
    });

    test("update multiple lines", ({expect, _}) => {
      let start = [|"[", "1,", "\"2\",", "3", "]", ""|];

      let endv = [|"[", "\"1\",", "2,", "\"3\"", "]"|];

      let jsonParser = Parser.json();
      let (_, baseline) = ArrayParser.parse(jsonParser, None, start);

      let update = [|"\"1\",", "2,", "\"3\""|];
      let delta = ArrayParser.Delta.create(baseline, 1, 4, update);

      let (tree, _) = ArrayParser.parse(jsonParser, Some(delta), endv);

      let node = Tree.getRootNode(tree);
      let ret = Node.toString(node);
      prerr_endline("RET: " ++ ret);
      expect.string(ret).toEqual(
        "(value (array (string (string_content)) (number) (string (string_content))))",
      );
    });

    let tokenRangeMatches = (~token, ~range: Range.t, ()) => {
      let startPosition = Syntax.Token.getPosition(token);
      let endPosition = Syntax.Token.getEndPosition(token);

      range.start.line == startPosition.line
      && range.start.column == startPosition.column
      && range.stop.line == endPosition.line
      && range.stop.column == endPosition.column;
    };
    test("token positions are preserved when deleting a line", ({expect, _}) => {
      let start = [|"[", "", "]"|];

      let endv = [|"[", "]"|];

      let jsonParser = Parser.json();
      let (_, baseline) = ArrayParser.parse(jsonParser, None, start);

      let update = [||];
      let delta = ArrayParser.Delta.create(baseline, 1, 2, update);

      let (tree, _) = ArrayParser.parse(jsonParser, Some(delta), endv);

      let node = Tree.getRootNode(tree);
      let range =
        Range.create(
          ~start=Location.create(~line=Index.zero, ~column=Index.zero),
          ~stop=Location.create(~line=Index.(zero + 3), ~column=Index.zero),
        );
      prerr_endline("-----START-------");
      let getTokenName = Syntax.createArrayTokenNameResolver(endv);
      let tokens = Syntax.getTokens(~getTokenName, ~range, node);

      // Validate tokens aren't shifted when deleting a row
      let leftBracket = List.nth(tokens, 0);
      expect.bool(
        tokenRangeMatches(
          ~token=leftBracket,
          ~range=
            Range.create(
              ~start=Location.create(~line=Index.zero, ~column=Index.zero),
              ~stop=
                Location.create(~line=Index.zero, ~column=Index.(zero + 1)),
            ),
          (),
        ),
      ).
        toBe(
        true,
      );

      expect.string(Syntax.Token.getName(leftBracket)).toEqual("\"[\"");

      let rightBracket = List.nth(tokens, 1);
      expect.bool(
        tokenRangeMatches(
          ~token=rightBracket,
          ~range=
            Range.create(
              ~start=
                Location.create(~line=Index.(zero + 1), ~column=Index.zero),
              ~stop=
                Location.create(
                  ~line=Index.(zero + 1),
                  ~column=Index.(zero + 1),
                ),
            ),
          (),
        ),
      ).
        toBe(
        true,
      );
      expect.string(Syntax.Token.getName(rightBracket)).toEqual("\"]\"");
      List.iter(t => prerr_endline(Syntax.Token.show(t)), tokens);
    });
    test("token positions are preserved when adding a line", ({expect, _}) => {
      let start = [|"[", "]"|];

      let endv = [|"[", "", "]"|];

      let jsonParser = Parser.json();
      let (_, baseline) = ArrayParser.parse(jsonParser, None, start);

      let update = [|""|];
      let delta = ArrayParser.Delta.create(baseline, 1, 1, update);

      let (tree, _) = ArrayParser.parse(jsonParser, Some(delta), endv);

      let node = Tree.getRootNode(tree);
      let getTokenName = _ => "";
      let range =
        Range.create(
          ~start=Location.create(~line=Index.zero, ~column=Index.zero),
          ~stop=Location.create(~line=Index.(zero + 3), ~column=Index.zero),
        );
      prerr_endline("-----START-------");
      let tokens = Syntax.getTokens(~getTokenName, ~range, node);

      // Validate tokens aren't shifted when deleting a row
      let leftBracket = List.nth(tokens, 0);
      expect.bool(
        tokenRangeMatches(
          ~token=leftBracket,
          ~range=
            Range.create(
              ~start=Location.create(~line=Index.zero, ~column=Index.zero),
              ~stop=
                Location.create(~line=Index.zero, ~column=Index.(zero + 1)),
            ),
          (),
        ),
      ).
        toBe(
        true,
      );

      let rightBracket = List.nth(tokens, 1);
      expect.bool(
        tokenRangeMatches(
          ~token=rightBracket,
          ~range=
            Range.create(
              ~start=
                Location.create(~line=Index.(zero + 2), ~column=Index.zero),
              ~stop=
                Location.create(
                  ~line=Index.(zero + 2),
                  ~column=Index.(zero + 1),
                ),
            ),
          (),
        ),
      ).
        toBe(
        true,
      );
      List.iter(t => prerr_endline(Syntax.Token.show(t)), tokens);
    });
    test(
      "token positions are preserved when modifying a line", ({expect, _}) => {
      let start = [|"[", "", "]"|];

      let endv = [|"[", "a", "]"|];

      let jsonParser = Parser.json();
      let (_, baseline) = ArrayParser.parse(jsonParser, None, start);

      let update = [|"a"|];
      let delta = ArrayParser.Delta.create(baseline, 1, 2, update);

      let (tree, _) = ArrayParser.parse(jsonParser, Some(delta), endv);

      let node = Tree.getRootNode(tree);
      let range =
        Range.create(
          ~start=Location.create(~line=Index.zero, ~column=Index.zero),
          ~stop=Location.create(~line=Index.(zero + 3), ~column=Index.zero),
        );
      let getTokenName = Syntax.createArrayTokenNameResolver(endv);
      prerr_endline("-----START-------");
      let tokens = Syntax.getTokens(~getTokenName, ~range, node);

      // Validate tokens aren't shifted when deleting a row
      let leftBracket = List.nth(tokens, 0);
      expect.bool(
        tokenRangeMatches(
          ~token=leftBracket,
          ~range=
            Range.create(
              ~start=Location.create(~line=Index.zero, ~column=Index.zero),
              ~stop=
                Location.create(~line=Index.zero, ~column=Index.(zero + 1)),
            ),
          (),
        ),
      ).
        toBe(
        true,
      );

      expect.string(Syntax.Token.getName(leftBracket)).toEqual("\"[\"");

      let rightBracket = List.nth(tokens, 2);
      expect.bool(
        tokenRangeMatches(
          ~token=rightBracket,
          ~range=
            Range.create(
              ~start=
                Location.create(~line=Index.(zero + 2), ~column=Index.zero),
              ~stop=
                Location.create(
                  ~line=Index.(zero + 2),
                  ~column=Index.(zero + 1),
                ),
            ),
          (),
        ),
      ).
        toBe(
        true,
      );
      expect.string(Syntax.Token.getName(rightBracket)).toEqual("\"]\"");
      List.iter(t => prerr_endline(Syntax.Token.show(t)), tokens);
    });
    test("regression test: multiple delta updates", ({expect, _}) => {
      let jsonParser = Parser.json();
      let start = [|"[", "]"|];
      let (_, baseline) = ArrayParser.parse(jsonParser, None, start);

      let delta1 = [|"\"a\",", "\"b\","|];
      let end1 = [|"[", "\"a\",", "\"b\",", "]"|];

      let delta = ArrayParser.Delta.create(baseline, 1, 1, delta1);
      let (_, baseline) = ArrayParser.parse(jsonParser, Some(delta), end1);

      let delta2 = [|""|];
      let end2 = [|"[", "\"a\",", "\"b\",", "", "]"|];

      let delta = ArrayParser.Delta.create(baseline, 3, 3, delta2);
      let (_, baseline) = ArrayParser.parse(jsonParser, Some(delta), end2);

      let delta3 = [|"\""|];
      let end3 = [|"[", "\"a\",", "\"b\",", "\"", "]"|];

      let delta = ArrayParser.Delta.create(baseline, 3, 4, delta3);
      let (tree, _) = ArrayParser.parse(jsonParser, Some(delta), end3);

      /*let delta4 = [|"\"c"|];
        let end4 = [|"[", "\"abc\",", "\"c", "]"|];

        let delta = ArrayParser.Delta.create(baseline, 2, 3, delta4);
        let (_, baseline) = ArrayParser.parse(jsonParser, Some(delta), end4);

        let delta5 = [|"\"\",", "\"d"|];
        let end5 = [|"[", "\"abc\",", "\"\",", "\"d", "]"|];

        let delta = ArrayParser.Delta.create(baseline, 2, 3, delta5);
        let (_, baseline) = ArrayParser.parse(jsonParser, Some(delta), end5);

        let delta6 = [|"e"|];
        let end6 = [|"[", "\"abc\",", "\"\",", "\"d", "e", "]"|];

        let delta = ArrayParser.Delta.create(baseline, 3, 3, delta6);
        let (tree, _) = ArrayParser.parse(jsonParser, Some(delta), end6); */

      let node = Tree.getRootNode(tree);
      let rangeLine2 =
        Range.create(
          ~start=Location.create(~line=Index.zero, ~column=Index.zero),
          ~stop=Location.create(~line=Index.(zero + 6), ~column=Index.zero),
        );

      let getTokenName = Syntax.createArrayTokenNameResolver(end3);
      let tokens = Syntax.getTokens(~getTokenName, ~range=rangeLine2, node);
      // There should be these tokens at this point:
      // Token(0,0 - 0,1:(0:array).(0:value)|"[")
      // Token(1,0 - 1,1:(0:string).(0:array).(0:value)|""")
      // Token(1,1 - 1,2:(0:string_content).(0:string).(0:array).(0:value)|"a")
      // Token(1,2 - 1,3:(0:string).(0:array).(0:value)|""")
      // Token(1,3 - 1,4:(0:array).(0:value)|",")
      // Token(2,0 - 2,1:(1:string).(0:array).(0:value)|""")
      // Token(2,1 - 2,2:(0:string_content).(1:string).(0:array).(0:value)|"b")
      // Token(2,2 - 2,3:(1:string).(0:array).(0:value)|""")
      // Token(2,3 - 2,4:(0:array).(0:value)|",")
      // Token(3,0 - 3,1:(2:string).(0:array).(0:value)|""")
      // Token(3,1 - 3,1:(2:string).(0:array).(0:value)|)
      // Token(4,0 - 4,1:(0:array).(0:value)|"]")

      //List.iter((t) => prerr_endline (Syntax.Token.show(t)), tokens);

      // With the byte offest bug, we'd instead crash when resolving tokens
      expect.int(List.length(tokens)).toBe(12);
    });
  });
});
