open TestFramework;

open Treesitter;

describe("Node", ({describe, _}) => {
  let jsonParser = Parser.json();
  let (tree, _) =
    ArrayParser.parse(jsonParser, None, [|"[1,", "\"2\"", "]"|]);
  let simpleNode = Tree.getRootNode(tree);

  let (tree, _) = ArrayParser.parse(jsonParser, None, [|"[,]"|]);
  let errorNode = Tree.getRootNode(tree);
  // "(value (array (number) (string (string_content))))",

  describe("getIndex / getNamedIndex", ({test, _}) => {
    test("getNamedIndex returns correct values", ({expect, _}) => {
      let firstChild = Node.getNamedChild(simpleNode, 0);
      let child1 = Node.getNamedChild(firstChild, 0);
      let child2 = Node.getNamedChild(firstChild, 1);

      expect.int(Node.getNamedIndex(child1)).toBe(0);
      expect.int(Node.getNamedIndex(child2)).toBe(1);
    });
    test("getIndex returns correct values", ({expect, _}) => {
      let firstChild = Node.getNamedChild(simpleNode, 0);
      let child1 = Node.getChild(firstChild, 0);
      let child2 = Node.getChild(firstChild, 1);

      expect.int(Node.getIndex(child1)).toBe(0);
      expect.int(Node.getIndex(child2)).toBe(1);

      expect.int(Node.getNamedIndex(child1)).toBe(0);
      expect.int(Node.getNamedIndex(child2)).toBe(0);
    });
  });

  describe("getDescendantForPointRange", ({test, _}) => {
    test("gets single line", ({expect, _}) => {
      let line2Node = Node.getDescendantForPointRange(simpleNode, 1, 0, 1, 3);
      let ret = Node.toString(line2Node);

      let startPoint = Node.getStartPoint(line2Node);
      let endPoint = Node.getEndPoint(line2Node);

      expect.int((startPoint.line :> int)).toBe(1);
      expect.int((startPoint.column :> int)).toBe(0);

      expect.int((endPoint.line :> int)).toBe(1);
      expect.int((endPoint.column :> int)).toBe(3);

      prerr_endline("RET: " ++ ret);
      expect.string(ret).toEqual("(string (string_content))");
    });
    describe("getStartPoint / getEndPoint", ({test, _}) =>
      test("returns correct value for root", ({expect, _}) => {
        let startPoint = Node.getStartPoint(simpleNode);
        let endPoint = Node.getEndPoint(simpleNode);

        expect.int((startPoint.line :> int)).toBe(0);
        expect.int((startPoint.column :> int)).toBe(0);

        expect.int((endPoint.line :> int)).toBe(3);
        expect.int((endPoint.column :> int)).toBe(0);
      })
    );

    describe("isError / hasError", ({test, _}) => {
      test("hasError returns false for no errors", ({expect, _}) =>
        expect.bool(Node.hasError(simpleNode)).toBe(
          false,
          //expect.bool(Node.isError(simpleNode)).toBe(false);
        )
      );
      test("hasError turns true when there are errors", ({expect, _}) => {
        prerr_endline("ERROR: " ++ Node.toString(errorNode));
        expect.bool(Node.hasError(errorNode)).toBe(true);
      });
      test("isError returns true only for the error node", ({expect, _}) => {
        expect.bool(Node.isError(errorNode)).toBe(false);

        let firstChild = Node.getNamedChild(errorNode, 0);
        let firstGrandChild = Node.getNamedChild(firstChild, 0);

        expect.bool(Node.isError(firstChild)).toBe(false);
        expect.bool(Node.isError(firstGrandChild)).toBe(true);

        prerr_endline("GC: " ++ Node.toString(firstGrandChild));
      });
    });
  });
});
