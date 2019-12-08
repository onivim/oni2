/*
   TreesitterTokenizerJob.re

   TreesitterTokenizerJob is a BufferLineJob that figures out tokens
   per-line.
 */

open Oni_Core;

open Treesitter;
open TreeSitterScopes;

type context = {
  tree: Treesitter.Tree.t,
  lines: array(string),
  theme: TokenTheme.t,
  scopeConverter: TextMateConverter.t,
};

type output = list(ColorizedToken.t);

type t = BufferLineJob.t(context, output);

let isComplete = Job.isComplete;

let noTokens = [];
let getTokensForLine = (line: int, v: t) => {
  switch (BufferLineJob.getCompletedWork(line, v)) {
  | Some(v) => v
  | None => noTokens
  };
};

let notifyBufferUpdate = BufferLineJob.notifyBufferUpdate;

let updateTheme = (theme: TokenTheme.t, v: t) => {
  let oldContext = BufferLineJob.getContext(v);
  let newContext = {...oldContext, theme};

  BufferLineJob.clear(~newContext=Some(newContext), v);
};

let doWork = (context: context, line: int) => {
  let rootNode = Tree.getRootNode(context.tree);
  let range =
    Treesitter.Types.Range.create(
      ~startPosition=Treesitter.Types.Position.create(~line, ~column=0, ()),
      ~endPosition=
        Treesitter.Types.Position.create(~line=line + 1, ~column=0, ()),
      (),
    );

  let getTokenName = Syntax.createArrayTokenNameResolver(context.lines);
  let tokens = Syntax.getTokens(~getTokenName, ~range, rootNode);

  List.map(
    curr => {
      let (p: Treesitter.Types.Position.t, _, scopes, token) = curr;
      let tmScope =
        TextMateConverter.getTextMateScope(
          ~token,
          ~path=scopes,
          context.scopeConverter,
        );

      let resolvedColor = TokenTheme.match(context.theme, tmScope);

      //let line = p.line;
      let col = p.column;

      ColorizedToken.create(
        ~index=col,
        ~backgroundColor=Revery.Color.hex(resolvedColor.background),
        ~foregroundColor=Revery.Color.hex(resolvedColor.foreground),
        (),
      );
    },
    tokens,
  );
};

let create = context => {
  BufferLineJob.create(
    ~name="TreesitterTokenizerJob",
    ~initialContext=context,
    ~f=doWork,
  );
};
