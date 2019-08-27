/*
 SyntaxHighlights.re
 */

open Revery;
open Oni_Core.Types;

module type SyntaxHighlight = {
  type t;

  type treeSitterScopeMapperFactory = unit => TreeSitterScopes.TextMateConverter.t;

  let hasPendingWork: t => bool;
  let doChunkOfWork: t => t;

  let create: (~getTreeSitterScopeMapper: treeSitterScopeMapperFactory, array(string)) => t;

  let getTokenColors: (t, int) => list(ColorizedToken2.t);

  let update: (BufferUpdate.t, array(string), t) => t;
};

Printexc.record_backtrace(true);

module TreeSitterSyntaxHighlight: SyntaxHighlight = {
  open Oni_Core;
  open Treesitter;
  open TreeSitterScopes;
  
  type treeSitterScopeMapperFactory = unit => TreeSitterScopes.TextMateConverter.t;
  
  type t = {
    parser: Parser.t,
    tree: Tree.t,
    lastBaseline: ArrayParser.Baseline.t,
    lineToTokens: IntMap.t(list(ColorizedToken2.t)),
    scopeConverter: TextMateConverter.t,
  };

  let create = (~getTreeSitterScopeMapper, lines: array(string)) =>  {
    let parser = Parser.json();
    let (tree, baseline) = ArrayParser.parse(parser, None, lines);

    let rootNode = Tree.getRootNode(tree);
    let scopeConverter = getTreeSitterScopeMapper();

    let i = ref(0);

    let getTokenName = Syntax.createArrayTokenNameResolver(lines);
    let lineCount = Array.length(lines);
    while((i^)  < lineCount) {
      let idx = i^;
      let range = TreeSitter.Types.Range.create(
          ~startPosition=Treesitter.Types.Position.create(
              ~line=idx,
              ~column=0, ()),
          ~endPosition=Treesitter.Types.Position.create(
            ~line=idx+1,
            ~column=0,
            ()),
            ());

      let tokens = Syntax.getTokens(~getTokenName, ~range, rootNode);

      let someOrNone = v => switch(v) {
      | Some(v) => ("Some(" ++ v ++ ")");
      | None => ("None");
      };
      print_endline ("Tokens for line: " ++ string_of_int(idx));
      List.iter((t) => {
        let (p, scopes, token) = t;
        print_endline("Position: " 
          ++ Treesitter.Types.Position.show(p)
          ++ "| Syntax: " ++ Syntax.Token.show(t)
          ++ "| Scope: "
        ++ someOrNone(TextMateConverter.getTextMateScope(~token, ~path=scopes, scopeConverter)));
      }, tokens);

      incr(i);
    };

    {
      parser,
      tree,
      lastBaseline: baseline,
      lineToTokens: IntMap.empty,
      scopeConverter,
    }
  };

  let hasPendingWork = (_) => false;
  let doChunkOfWork = (v) => v;

  let getTokenColors = (_, _) => [
    ColorizedToken2.create(
      ~index=0,
      ~backgroundColor=Colors.red,
      ~foregroundColor=Colors.red,
      (),
    ),
    ColorizedToken2.create(
      ~index=10,
      ~backgroundColor=Colors.black,
      ~foregroundColor=Colors.white,
      (),
    ),
    ColorizedToken2.create(
      ~index=20,
      ~backgroundColor=Colors.black,
      ~foregroundColor=Colors.blue,
      (),
    ),
  ];

  let update = (_, _, v) => v;
};

module SyntaxHighlights = {
  type t =
    | TreeSitter(TreeSitterSyntaxHighlight.t)
    | None;

  let default = None;

  let create = (~getTreeSitterScopeMapper, lines: array(string)) => {
    let ts = TreeSitterSyntaxHighlight.create(~getTreeSitterScopeMapper, lines);
    TreeSitter(ts);
  };

  let getTokensForLine = (v: t, line: int) => {
    switch (v) {
    | TreeSitter(ts) => TreeSitterSyntaxHighlight.getTokenColors(ts, line);
    | _ => []
    }
  };
};
