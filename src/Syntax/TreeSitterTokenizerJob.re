/*
   TreesitterTokenizerJob.re

   TreesitterTokenizerJob is a BufferLineJob that figures out tokens
   per-line.
 */

open EditorCoreTypes;
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
let getUpdatedLines = BufferLineJob.getUpdatedLines;
let clearUpdatedLines = BufferLineJob.clearUpdatedLines;

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
    Range.{
      start: Location.{line: Index.fromZeroBased(line), column: Index.zero},
      stop:
        Location.{line: Index.fromZeroBased(line + 1), column: Index.zero},
    };

  let getTokenName = Syntax.createArrayTokenNameResolver(context.lines);
  let tokens = Syntax.getTokens(~getTokenName, ~range, rootNode);

  List.map(
    curr => {
      let (loc: Location.t, _, scopes, token) = curr;
      let tmScope =
        TextMateConverter.getTextMateScope(
          ~token,
          ~path=scopes,
          context.scopeConverter,
        );

      let {background, foreground, bold, italic}: Textmate.ThemeScopes.ResolvedStyle.t = TokenTheme.match(pending.theme, scopes);

      ColorizedToken.create(
        ~index=Index.toZeroBased(loc.column),
        ~backgroundColor=Revery.Color.hex(background),
        ~foregroundColor=Revery.Color.hex(foreground),
        ~bold,
        ~italic,
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
