/*
   TextmateTokenizerJob.re
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

module Time = Revery_Core.Time;
module Log = (val Log.withNamespace("Oni2.Syntax.TextmateTokenizerJob"));

module Internal = {
  let hexToColorCache = Hashtbl.create(128);

  let hexToColor = (hex: string) => {
    switch (Hashtbl.find_opt(hexToColorCache, hex)) {
    | None =>
      let color = Revery.Color.hex(hex);
      Hashtbl.add(hexToColorCache, hex, color);
      color;
    | Some(color) => color
    };
  };
};

// open Textmate;

type pendingWork = {
  lines: array(string),
  currentLine: int,
  currentVersion: int,
  tokenizer: Textmate.Tokenizer.t,
  theme: TokenTheme.t,
  scope: string,
  hasRun: bool,
};

type lineInfo = {
  tokens: list(ColorizedToken.t),
  scopeStack: Textmate.ScopeStack.t,
  version: int,
};

type completedWork = {
  tokens: IntMap.t(lineInfo),
  latestLines: list(int),
};

let initialCompletedWork = {tokens: IntMap.empty, latestLines: []};

type t = Job.t(pendingWork, completedWork);

let getTokenColors = (line: int, v: t) => {
  let completed = Job.getCompletedWork(v).tokens;
  switch (IntMap.find_opt(line, completed)) {
  | Some({tokens, _}) => tokens
  | None => []
  };
};

let clearUpdatedLines = (tm: t) => {
  let isComplete = Job.isComplete(tm);
  let f = (p: pendingWork, c: completedWork) => {
    (isComplete, p, {...c, latestLines: []});
  };

  Job.map(f, tm);
};

let onTheme = (theme: TokenTheme.t, v: t) => {
  let f = (p: pendingWork, _c: completedWork) => {
    let newPendingWork = {...p, theme, currentLine: 0};

    let newCompletedWork = initialCompletedWork;

    (false, newPendingWork, newCompletedWork);
  };

  Job.map(f, v);
};

let onBufferUpdate = (bufferUpdate: BufferUpdate.t, lines, v: t) => {
  let startPos = Index.toZeroBased(bufferUpdate.startLine);
  let endPos = Index.toZeroBased(bufferUpdate.endLine);

  let f = (p: pendingWork, c: completedWork) => {
    (
      false,
      {
        ...p,
        lines,
        currentLine: min(startPos, p.currentLine),
        currentVersion: bufferUpdate.version,
      },
      {
        ...c,
        tokens:
          IntMap.shift(
            ~default=
              prev =>
                switch (prev) {
                | None => None
                | Some({scopeStack, _}) =>
                  Some({tokens: [], scopeStack, version: (-1)})
                },
            ~startPos,
            ~endPos,
            ~delta=Array.length(bufferUpdate.lines),
            c.tokens,
          ),
      },
    );
  };

  Job.map(f, v);
};

exception NoWhitespaceException;

let doWork = (pending: pendingWork, completed: completedWork) => {
  let currentLine = pending.currentLine;

  if (currentLine >= Array.length(pending.lines)) {
    (true, pending, completed);
  } else {
    // Check if there are scope stacks from the previous line
    let scopes =
      switch (IntMap.find_opt(currentLine - 1, completed.tokens)) {
      | None => None
      | Some(v) => Some(v.scopeStack)
      };

    Log.tracef(m => m("Tokenizing line: %i", currentLine));

    let line = pending.lines[currentLine] ++ "\n";

    // Get new tokens & scopes
    let (tokens, scopes) =
      Textmate.Tokenizer.tokenize(
        ~lineNumber=currentLine,
        ~scopeStack=scopes,
        ~scope=pending.scope,
        pending.tokenizer,
        line,
      );

    let isWhitespaceOnly = (startIndex, endIndex) => StringEx.forAll(~start=startIndex, ~stop=endIndex, 
      ~f=StringEx.isSpace,
      line);

    let tokens =
      tokens
      |> List.filter(({position, length, _}: Textmate.Token.t) => !isWhitespaceOnly(position, position+length))
      |> List.map(token => {
           let {position, scopes, _}: Textmate.Token.t = token;
           let combinedScopes = scopes |> String.concat(" ") |> String.trim;

           let resolvedColor =
             TokenTheme.match(pending.theme, combinedScopes);

           ColorizedToken.create(
             ~index=position,
             ~backgroundColor=Internal.hexToColor(resolvedColor.background),
             ~foregroundColor=Internal.hexToColor(resolvedColor.foreground),
             ~syntaxScope=SyntaxScope.ofScopes(scopes),
             (),
           );
         });

    let newLineInfo = {
      tokens,
      scopeStack: scopes,
      version: pending.currentVersion,
    };

    let tokens =
      IntMap.update(
        currentLine,
        prev =>
          switch (prev) {
          | None => Some(newLineInfo)
          | Some(_) => Some(newLineInfo)
          },
        completed.tokens,
      );

    let nextLine = currentLine + 1;
    let isComplete = nextLine >= Array.length(pending.lines);

    (
      isComplete,
      {...pending, hasRun: true, currentLine: nextLine},
      {tokens, latestLines: [currentLine, ...completed.latestLines]},
    );
  };
};

let create = (~scope, ~theme, ~grammarRepository, lines) => {
  let tokenizer =
    Textmate.Tokenizer.create(~repository=grammarRepository, ());
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
    ~budget=Time.ms(8),
    ~f=doWork,
    p,
  );
};
