/*
 TreeSitterSyntaxHighlighting.re
 */

open Oni_Core;
open Oni_Core.Types;
open Treesitter;
open TreeSitterScopes;

type treeSitterScopeMapperFactory =
  unit => TreeSitterScopes.TextMateConverter.t;

type t = {
  parser: Parser.t,
  tree: Tree.t,
  lastBaseline: ArrayParser.Baseline.t,
  lastLines: array(string),
  scopeConverter: TextMateConverter.t,
  theme: TextMateTheme.t,
};
let someOrNone = v =>
  switch (v) {
  | Some(v) => v
  | None => ""
  };

let scopesToStrings = (scopes: list(TreeSitter.Syntax.scope)) => {
  List.map(
    v => {
      let (_, s) = v;
      s;
    },
    scopes,
  );
};

let create = (~theme, ~getTreeSitterScopeMapper, lines: array(string)) => {
  let parser = Parser.c();
  let (tree, baseline) = ArrayParser.parse(parser, None, lines);

  //let rootNode = Tree.getRootNode(tree);
  let scopeConverter = getTreeSitterScopeMapper();

  //let i = ref(0);

  {
    parser,
    tree,
    lastBaseline: baseline,
    lastLines: lines,
    scopeConverter,
    theme,
  };
};

Printexc.record_backtrace(true);

let hasPendingWork = _ => false;
let doChunkOfWork = v => v;

let getTokenColors = (v: t, line: int) => {
  let rootNode = Tree.getRootNode(v.tree);
  let range =
    TreeSitter.Types.Range.create(
      ~startPosition=Treesitter.Types.Position.create(~line, ~column=0, ()),
      ~endPosition=
        Treesitter.Types.Position.create(~line=line + 1, ~column=0, ()),
      (),
    );

  let getTokenName = Syntax.createArrayTokenNameResolver(v.lastLines);
  let tokens = Syntax.getTokens(~getTokenName, ~range, rootNode);

  List.map(
    curr => {
      let (p: Treesitter.Types.Position.t, _, scopes, token) = curr;
      let tmScope =
        TextMateConverter.getTextMateScope(
          ~token,
          ~path=scopes,
          v.scopeConverter,
        );
      let resolvedColor = TextMateTheme.match(v.theme, tmScope);

      //let line = p.line;
      let col = p.column;

      ColorizedToken2.create(
        ~index=col,
        ~backgroundColor=resolvedColor.background,
        ~foregroundColor=resolvedColor.foreground,
        (),
      );
    },
    tokens,
  );
};

let update = (~bufferUpdate: BufferUpdate.t, ~lines: array(string), v: t) => {
  let { parser, lastBaseline, _ } = v;
  let start1 = Unix.gettimeofday();
  let delta = TreeSitter.ArrayParser.Delta.create(
      lastBaseline, 
      Index.toInt0(bufferUpdate.startLine),
      Index.toInt0(bufferUpdate.endLine),
      bufferUpdate.lines,
  );

  let end1 = Unix.gettimeofday();

  
  let start2 = Unix.gettimeofday();
  let (tree, newBaseline) = TreeSitter.ArrayParser.parse(parser, Some(delta), lines);
  let end2 = Unix.gettimeofday();


  print_endline ("PERF: Delta creation: " ++ string_of_float(end1 -. start1));
  print_endline ("PERF: Parse: " ++ string_of_float(end2 -. start2));

  let ret: t = {
    ...v,
    parser,
    tree,
    lastBaseline: newBaseline,
    lastLines: lines,
  };
  let i = ref(0);
  let len = Array.length(lines);
  let range = TreeSitter.Types.Range.createi(
  ~startLine=0,
  ~startColumn=0,
  ~endLine=10,
  ~endColumn=0,
  ()
  );

  //let getTokenName = TreeSitter.Syntax.createArrayTokenNameResolver(lines);
  //let getTokenName = (_) => "";
  //let tokens = TreeSitter.Syntax.getTokens(getTokenName, range, TreeSitter.Tree.getRootNode(tree));
  // List.iter(t => print_endline(TreeSitter.Syntax.Token.show(t)), tokens);
  //while (i^  < min(10,len)) {
    // print_endline ("Line " ++ string_of_int(i^) ++ ": " ++ Array.get(lines, i^));
    //let _ = getTokenColors(ret, i^);
    //incr(i);
  //};
  ret;
};

