open TestFramework;

open Treesitter;

describe("Parser", ({describe, _}) => {
  describe("allocation", ({test, _}) => {
    test("finalizer gets called for parser", ({expect, _}) => {
      let jsonParser = Parser.json();
      let callCount = ref(0);
      Gc.finalise_last(() => incr(callCount), jsonParser);
      Gc.full_major();

      expect.int(callCount^).toBe(1);
    });

    test("finalizer gets called for tree", ({expect, _}) => {
      let jsonParser = Parser.json();
      let tree = Parser.parseString(jsonParser, "[1, \"a\", null]");

      let callCount = ref(0);
      Gc.finalise_last(() => incr(callCount), tree);
      Gc.full_major();

      expect.int(callCount^).toBe(1);
    });
  });
  describe("json", ({test, _}) =>
    test("simple parsing case", ({expect, _}) => {
      let jsonParser = Parser.json();
      let tree = Parser.parseString(jsonParser, "[1, \"2\"]");
      //let tree = Parser.parseString(jsonParser, "{ \"a\": 1 }");
      let node = Tree.getRootNode(tree);
      let ret = Node.toString(node);
      prerr_endline("RET: " ++ ret);
      expect.string(ret).toEqual(
        "(value (array (number) (string (string_content))))",
      );

      expect.int(Node.getChildCount(node)).toBe(1);
      expect.string(Node.getType(node)).toEqual("value");

      let arrayNode = Node.getChild(node, 0);
      expect.string(Node.getType(arrayNode)).toEqual("array");
      expect.int(Node.getNamedChildCount(arrayNode)).toBe(2);
      expect.int(Node.getChildCount(arrayNode)).toBe(5);

      let array0 = Node.getNamedChild(arrayNode, 0);
      let array1 = Node.getNamedChild(arrayNode, 1);
      expect.string(Node.getType(array0)).toEqual("number");
      expect.string(Node.getType(array1)).toEqual("string");
    })
  );
  describe("c", ({test, _}) =>
    test("basic parse case", ({expect, _}) => {
      let jsonParser = Parser.c();
      let tree = Parser.parseString(jsonParser, "int main() { return 1; }");
      //let tree = Parser.parseString(jsonParser, "{ \"a\": 1 }");
      let node = Tree.getRootNode(tree);
      let ret = Node.toString(node);
      prerr_endline("RET: " ++ ret);
      expect.string(ret).toEqual(
        "(translation_unit (function_definition type: (primitive_type) declarator: (function_declarator declarator: (identifier) parameters: (parameter_list)) body: (compound_statement (return_statement (number_literal)))))",
      );
    })
  );
});
