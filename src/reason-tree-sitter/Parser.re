/*
     Parser.re

     Stubs for bindings to the `TSParser` object
 */

type t;

// Parsers for particular syntaxes
external json: unit => t = "rets_parser_new_json";
external c: unit => t = "rets_parser_new_c";

// General parser methods
external parseString: (t, string) => Tree.t = "rets_parser_parse_string";

type readFunction = (int, int, int) => option(string);

external _parse: (t, option(Tree.t), readFunction) => Tree.t =
  "rets_parser_parse";

let _parse_read_fn: ref(readFunction) = ref((_, _, _) => None);

let _parse_read = (byteOffset: int, line: int, col: int) => {
  _parse_read_fn^(byteOffset, line, col);
};

let parse = (parser: t, tree: option(Tree.t), readFunction) => {
  _parse_read_fn := readFunction;
  _parse(parser, tree, _parse_read);
};

Callback.register("rets__parse_read", _parse_read);
