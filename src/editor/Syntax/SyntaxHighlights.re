/*
 SyntaxHighlights.re
 */

open Oni_Core.Types;

module TreeSitterSyntaxHighlight = {
  open Oni_Core;
  open Treesitter;
  open TreeSitterScopes;
  
  type treeSitterScopeMapperFactory = unit => TreeSitterScopes.TextMateConverter.t;
  
  type t = {
    parser: Parser.t,
    tree: Tree.t,
    lastBaseline: ArrayParser.Baseline.t,
    lastLines: array(string),
    lineToTokens: IntMap.t(list(ColorizedToken2.t)),
    scopeConverter: TextMateConverter.t,
    theme: TextMateTheme.t,
  };
  let someOrNone = v => switch(v) {
  | Some(v) => v;
  | None => "";
  };

  let scopesToStrings = (scopes: list(TreeSitter.Syntax.scope)) => {
    List.map((v) => {
      let (_, s) = v;
      s
    }, scopes);
  };

  let create = (~theme, ~getTreeSitterScopeMapper, lines: array(string)) =>  {
    let parser = Parser.json();
    let (tree, baseline) = ArrayParser.parse(parser, None, lines);

    let rootNode = Tree.getRootNode(tree);
    let scopeConverter = getTreeSitterScopeMapper();

    let i = ref(0);

    let getTokenName = Syntax.createArrayTokenNameResolver(lines);
    let lineCount = Array.length(lines);

    let lineToTokensRef = ref(IntMap.empty);
    while((i^)  < 10) {
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
      print_endline ("Tokens for line: " ++ string_of_int(idx));
      List.iteri((idx, t) => {
            let (p, scopes, token) = t;
            let tmScope = 
              TextMateConverter.getTextMateScope(~token, ~path=scopesToStrings(scopes), scopeConverter);
            let resolvedColor = TextMateTheme.match(theme, tmScope);
            print_endline("Position: " 
              ++ Treesitter.Types.Position.show(p)
              ++ "\n | Syntax: " ++ Syntax.Token.show(t)
              ++ "\n | Scope: " ++ tmScope
            ++ "\n | Foreground" ++ Revery.Color.show(resolvedColor.foreground));
        }, tokens);

      incr(i);
    };


    {
      parser,
      tree,
      lastBaseline: baseline,
      lastLines: lines,
      lineToTokens: lineToTokensRef^,
      scopeConverter,
      theme,
    }
  };

  let hasPendingWork = (_) => false;
  let doChunkOfWork = (v) => v;

  let getTokenColors = (v: t, line: int) => {
      let rootNode = Tree.getRootNode(v.tree);
      let range = TreeSitter.Types.Range.create(
          ~startPosition=Treesitter.Types.Position.create(
              ~line,
              ~column=0, ()),
          ~endPosition=Treesitter.Types.Position.create(
            ~line=line+1,
            ~column=0,
            ()),
            ());

    let getTokenName = Syntax.createArrayTokenNameResolver(v.lastLines);
    let tokens = Syntax.getTokens(~getTokenName, ~range, rootNode);

    List.map((curr) => {
        let (p: Treesitter.Types.Position.t, scopes, token) = curr;
        let tmScope = TextMateConverter.getTextMateScope(~token, ~path=scopesToStrings(scopes), v.scopeConverter);
        let resolvedColor = TextMateTheme.match(v.theme, tmScope);

        let line = p.line;
        let col = p.column;

        ColorizedToken2.create(
          ~index=col,
          ~backgroundColor=resolvedColor.background,
          ~foregroundColor=resolvedColor.foreground,
          ()
        );
    }, tokens);
  };

  let update = (_, _, v) => v;
};

module SyntaxHighlights = {
  type t =
    | TreeSitter(TreeSitterSyntaxHighlight.t)
    | None;

  let default = None;

  let create = (~theme, ~getTreeSitterScopeMapper, lines: array(string)) => {
    let ts = TreeSitterSyntaxHighlight.create(~theme, ~getTreeSitterScopeMapper, lines);
    TreeSitter(ts);
  };

  let getTokensForLine = (v: t, line: int) => {
    switch (v) {
    | TreeSitter(ts) => TreeSitterSyntaxHighlight.getTokenColors(ts, line);
    | _ => []
    }
  };
};
