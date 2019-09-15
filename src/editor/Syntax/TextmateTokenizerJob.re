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
  grammar: option(Grammar.t),
  theme: Textmate.Theme.t,
  scope: string,
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
  let f = (p: pendingWork, c: completedWork) => {
    (
      false,
      {
        ...p,
        lines,
        currentLine:
          min(Index.toInt0(bufferUpdate.startLine), p.currentLine),
        currentVersion: bufferUpdate.version,
      },
      c,
    );
  };

  Job.map(f, v);
};

let doWork = (pending: pendingWork, completed: completedWork) => {
  let currentLine = pending.currentLine;

  if (currentLine >= Array.length(pending.lines)) {
    (true, pending, completed);
  } else {
    switch (pending.grammar) {
    | None => (true, pending, completed)
    | Some(grammar) =>
      // Check if there are scope stacks from the previous line
      let scopes =
        switch (IntMap.find_opt(currentLine - 1, completed)) {
        | None => Grammar.getScopeStack(grammar)
        | Some(v) => v.scopeStack
        };

      // Get new tokens & scopes
      let (tokens, scopes) =
        Grammar.tokenize(
          ~lineNumber=currentLine,
          ~grammar,
          ~scopes=Some(scopes),
          pending.lines[currentLine] ++ "\n",
        );

      // Filter tokens and get colors
      let filteredTokens =
        tokens
        |> List.map(token => {
             open Token;
             let scopes = token.scopes |> List.filter(s => s != pending.scope);
             (token.position, scopes);
           });

      let tokens =
        List.map(
          token => {
            let (position, scopes) = token;
            let scopes =
              scopes
              |> List.fold_left((prev, curr) => {curr ++ " " ++ prev}, "")
              |> String.trim;

            let resolvedColor = Textmate.Theme.match(pending.theme, scopes);

            let col = position;
            ColorizedToken2.create(
              ~index=col,
              ~backgroundColor=Revery.Color.hex(resolvedColor.background),
              ~foregroundColor=Revery.Color.hex(resolvedColor.foreground),
              (),
            );
          },
          filteredTokens,
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

      (isComplete, {...pending, currentLine: nextLine}, completed);
    };
  };
};

let create = (~scope, ~theme, ~grammar, lines) => {
  let p: pendingWork = {
    lines,
    currentLine: 0,
    currentVersion: (-1),
    grammar,
    theme,
    scope,
  };

  Job.create(
    ~name="TextmateTokenizerJob",
    ~initialCompletedWork,
    ~budget=Milliseconds(2.),
    ~f=doWork,
    p,
  );
};
