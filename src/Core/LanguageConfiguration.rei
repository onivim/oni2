/*
 * LanguageConfiguration.rei
 */

open Oniguruma;

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

  let decode: Json.decoder(t);
};

module BracketPair: {
  type t = {
    openPair: string,
    closePair: string,
  };

  let decode: Json.decoder(t);
};

type t = {
  autoCloseBefore: list(string),
  autoClosingPairs: list(AutoClosingPair.t),
  brackets: list(BracketPair.t),
  lineComment: option(string),
  blockComment: option((string, string)),
  increaseIndentPattern: option(OnigRegExp.t),
  decreaseIndentPattern: option(OnigRegExp.t),
};

let default: t;

let shouldIncreaseIndent:
  (~previousLine: string, ~beforePreviousLine: option(string), t) => bool;

let shouldDecreaseIndent: (~line: string, t) => bool;

let decode: Json.decoder(t);

let toVimAutoClosingPairs: (SyntaxScope.t, t) => Vim.AutoClosingPairs.t;

let toAutoIndent:
  (t, ~previousLine: string, ~beforePreviousLine: option(string)) =>
  Vim.AutoIndent.action;
