/*
 * LanguageConfiguration.rei
 */

module Json = Oni_Core.Json;
module SyntaxScope = Oni_Core.SyntaxScope;

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

type t = {
  autoCloseBefore: list(string),
  autoClosingPairs: list(AutoClosingPair.t),
  lineComment: option(string),
  blockComment: option((string, string)),
};

let default: t;

let decode: Json.decoder(t);

let toVimAutoClosingPairs: (SyntaxScope.t, t) => Vim.AutoClosingPairs.t;
