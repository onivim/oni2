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
  let parser = Parser.json();
  let (tree, baseline) = ArrayParser.parse(parser, None, lines);

  let rootNode = Tree.getRootNode(tree);
  let scopeConverter = getTreeSitterScopeMapper();

  let i = ref(0);

  {
    parser,
    tree,
    lastBaseline: baseline,
    lastLines: lines,
    scopeConverter,
    theme,
  };
};

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
      let (p: Treesitter.Types.Position.t, scopes, token) = curr;
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

let update = (_, _, v) => v;
