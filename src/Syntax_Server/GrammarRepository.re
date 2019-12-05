/*
 * GrammarRepository.re
 */

open Oni_Core;

module Ext = Oni_Extensions;

//type t = {scopeToGrammar: Core.StringMap.t(Textmate.Grammar.t)};

let ofLanguageInfo = (languageInfo: Ext.LanguageInfo.t) => {
  let scopeToGrammar: Hashtbl.t(string, Textmate.Grammar.t) =
    Hashtbl.create(32);

  let f = scope => {
    switch (Hashtbl.find_opt(scopeToGrammar, scope)) {
    | Some(v) => Some(v)
    | None =>
      switch (Ext.LanguageInfo.getGrammarPathFromScope(languageInfo, scope)) {
      | Some(grammarPath) =>
        Log.info("GrammarRepository - Loading grammar: " ++ grammarPath);
        let json = Yojson.Safe.from_file(grammarPath);
        let grammar = Textmate.Grammar.Json.of_yojson(json);

        switch (grammar) {
        | Ok(g) =>
          Hashtbl.add(scopeToGrammar, scope, g);
          Some(g);
        | Error(e) =>
          Log.error("Error parsing grammar: " ++ e);
          None;
        };
      | None => None
      }
    };
  };

  f;
};
