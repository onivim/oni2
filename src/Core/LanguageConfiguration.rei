/*
 * LanguageConfiguration.rei
 */

module AutoClosingPair: {
  type scopes =
    | String
    | Comment
    | Other(string);

  type t = {
    openPair: string,
    closePair: string,
    notIn: list(scopes),
  };
};

module BracketPair: {
  type t = {
    openPair: string,
    closePair: string,
  };
};

type t;

let default: t;

let brackets: t => list(BracketPair.t);
let lineComment: t => option(string);
let blockComment: t => option((string, string));

let isWordCharacter: (Uchar.t, t) => bool;

let shouldIncreaseIndent:
  (~previousLine: string, ~beforePreviousLine: option(string), t) => bool;

let shouldDecreaseIndent: (~line: string, t) => bool;

let decode: Json.decoder(t);

let toVimAutoClosingPairs: (SyntaxScope.t, t) => Vim.AutoClosingPairs.t;

let toAutoIndent:
  (t, ~previousLine: string, ~beforePreviousLine: option(string)) =>
  Vim.AutoIndent.action;
