/*
 NoopSyntaxHighlighter.re
 */

open Oni_Core;

open Revery;

type t = {
  lines: array(string),
  theme: TokenTheme.t,
  updatedLines: list(int),
  tokenColors: IntMap.t(list(ThemeToken.t)),
};

let hasPendingWork = _ => false;
let doWork = v => v;

let updateVisibleRanges = (_, v) => v;
let updateTheme = (theme, v) => {...v, theme};

let update = (~bufferUpdate: BufferUpdate.t, ~lines: array(string), v) => {
  ignore(bufferUpdate);

  let updatedLines = lines |> Array.to_list |> List.mapi((idx, _) => idx);

  let tokenColors: IntMap.t(list(ThemeToken.t)) =
    List.fold_left(
      (acc: IntMap.t(list(ThemeToken.t)), curr: int) => {
        let tokens = [
          ThemeToken.create(
            ~index=0,
            ~backgroundColor=Colors.black,
            ~foregroundColor=Colors.red,
            ~syntaxScope=SyntaxScope.none,
            (),
          ),
          ThemeToken.create(
            ~index=2,
            ~backgroundColor=Colors.black,
            ~foregroundColor=Colors.green,
            ~syntaxScope=SyntaxScope.none,
            (),
          ),
          ThemeToken.create(
            ~index=3,
            ~backgroundColor=Colors.black,
            ~foregroundColor=TokenTheme.getEntityColor(v.theme),
            ~syntaxScope=SyntaxScope.none,
            (),
          ),
          ThemeToken.create(
            ~index=4,
            ~backgroundColor=Colors.black,
            ~foregroundColor=TokenTheme.getCommentColor(v.theme),
            ~syntaxScope=SyntaxScope.none,
            (),
          ),
          ThemeToken.create(
            ~index=5,
            ~backgroundColor=Colors.black,
            ~foregroundColor=TokenTheme.getKeywordColor(v.theme),
            ~syntaxScope=SyntaxScope.none,
            (),
          ),
        ];
        IntMap.add(curr, tokens, acc);
      },
      IntMap.empty,
      updatedLines,
    );

  {...v, updatedLines, tokenColors};
};

let create = (~bufferUpdate, ~theme, lines) => {
  let noopHighlights = {
    theme,
    lines,
    updatedLines: [],
    tokenColors: IntMap.empty,
  };

  noopHighlights |> update(~bufferUpdate, ~lines);
};

let getUpdatedLines = v => v.updatedLines;
let clearUpdatedLines = v => {...v, updatedLines: []};

let getTokenColors = (v, line) => {
  switch (IntMap.find_opt(line, v.tokenColors)) {
  | None => []
  | Some(tokens) => tokens
  };
};
