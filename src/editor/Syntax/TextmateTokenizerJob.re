/*
   TextmateTokenizerJob.re
 */

open Oni_Core;
open Oni_Core.Types;

open Textmate;

type pendingWork = {
  lines: array(string),
  currentLine: int,
  currentVersion: int,
  tokenizer: Tokenizer.t,
  theme: TokenTheme.t,
  scope: string,
  hasRun: bool,
};

type lineInfo = {
  tokens: list(ColorizedToken2.t),
  scopeStack: ScopeStack.t,
  version: int,
};

type completedWork = IntMap.t(lineInfo);
let initialCompletedWork = IntMap.empty;

type t = Job.t(pendingWork, completedWork);

let getTokenColors = (line: int, v: t) => {
  let completed = Job.getCompletedWork(v);
  switch (IntMap.find_opt(line, completed)) {
  | Some({tokens, _}) => tokens
  | None => []
  };
};

let onBufferUpdate = (bufferUpdate: BufferUpdate.t, lines, v: t) => {
  let startPos = Index.toInt0(bufferUpdate.startLine);
  let endPos = Index.toInt0(bufferUpdate.endLine);

  let f = (p: pendingWork, c: completedWork) => {
    (
      false,
      {
        ...p,
        lines,
        currentLine:
          min(startPos, p.currentLine),
        currentVersion: bufferUpdate.version,
      },
      IntMap.shift(~default={(prev) => switch(prev) {
      | None => None
      | Some({scopeStack, _}) => Some({tokens: [], scopeStack, version: -1});
      }}, ~startPos, ~endPos, 
        ~delta=Array.length(lines),
      c)
    );
  };

  Job.map(f, v);
};

let doWork = (pending: pendingWork, completed: completedWork) => {
  let currentLine = pending.currentLine;

  if (currentLine >= Array.length(pending.lines)) {
    (true, pending, completed);
  } else {
    // Check if there are scope stacks from the previous line
    let scopes =
      switch (IntMap.find_opt(currentLine - 1, completed)) {
      | None => None
      | Some(v) => Some(v.scopeStack)
      };

    // Get new tokens & scopes
    let (tokens, scopes) =
      Tokenizer.tokenize(
        ~lineNumber=currentLine,
        ~scopeStack=scopes,
        ~scope=pending.scope,
        pending.tokenizer,
        pending.lines[currentLine] ++ "\n",
      );

    let tokens =
      List.map(
        token => {
          let {position, scopes, _}: Token.t = token;
          let scopes =
            scopes
            |> List.fold_left((prev, curr) => {curr ++ " " ++ prev}, "")
            |> String.trim;

          let resolvedColor = TokenTheme.match(pending.theme, scopes);

          let col = position;
          ColorizedToken2.create(
            ~index=col,
            ~backgroundColor=Revery.Color.hex(resolvedColor.background),
            ~foregroundColor=Revery.Color.hex(resolvedColor.foreground),
            (),
          );
        },
        tokens,
      );

    let newLineInfo = {
      tokens,
      scopeStack: scopes,
      version: pending.currentVersion,
    };

    let completed =
      IntMap.update(
        currentLine,
        prev =>
          switch (prev) {
          | None => Some(newLineInfo)
          | Some(_) => Some(newLineInfo)
          },
        completed,
      );

    let nextLine = currentLine + 1;
    let isComplete = nextLine >= Array.length(pending.lines);

    (
      isComplete,
      {...pending, hasRun: true, currentLine: nextLine},
      completed,
    );
  };
};

let create = (~scope, ~theme, ~grammarRepository, lines) => {
  let tokenizer = Tokenizer.create(~repository=grammarRepository, ());
  let p: pendingWork = {
    lines,
    currentLine: 0,
    currentVersion: (-1),
    tokenizer,
    theme,
    scope,
    hasRun: false,
  };

  Job.create(
    ~name="TextmateTokenizerJob",
    ~initialCompletedWork,
    ~budget=Milliseconds(2.),
    ~f=doWork,
    p,
  );
};
