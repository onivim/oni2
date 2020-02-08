/*
 * LanguageConfiguration.rei
 */

open Oni_Core.Utility;
module Json = Oni_Core.Json;

module AutoClosingPair: {
  type scopes =
    | String
    | Comment
    | Other(string)
    | None;

  type t = {
    openPair: string,
    closePair: string,
    notIn: list(scopes),
  };
};

type t = {
  autoCloseBefore: list(string),
  autoClosingPairs: list(AutoClosingPair.t),
};

let default: t;

let decode: Json.decoder(t);

let toVimAutoClosingPairs:
  (AutoClosingPair.scopes, t) => Vim.AutoClosingPairs.t;
