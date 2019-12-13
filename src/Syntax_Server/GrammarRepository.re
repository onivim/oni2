/*
 * GrammarRepository.re
 */

module Ext = Oni_Extensions;

type t = {
  scopeToGrammar: Hashtbl.t(string, Textmate.Grammar.t),
  languageInfo: Ext.LanguageInfo.t,
  log: string => unit,
};

let create = (~log=_ => (), languageInfo) => {
  log,
  scopeToGrammar: Hashtbl.create(32),
  languageInfo,
};

let empty = create(Ext.LanguageInfo.empty);

let getGrammar = (~scope: string, gr: t) => {
  switch (Hashtbl.find_opt(gr.scopeToGrammar, scope)) {
  | Some(v) => Some(v)
  | None =>
    gr.log("getGrammar - querying language info");
    switch (Ext.LanguageInfo.getGrammarPathFromScope(gr.languageInfo, scope)) {
    | Some(grammarPath) =>
      gr.log("Loading grammar from: " ++ grammarPath);
      let json = Yojson.Safe.from_file(grammarPath);
      let resultGrammar = Textmate.Grammar.Json.of_yojson(json);

      switch (resultGrammar) {
      | Ok(grammar) =>
        gr.log("Grammar loaded successfully");
        Hashtbl.add(gr.scopeToGrammar, scope, grammar);
        Some(grammar);
      | Error(e) =>
        gr.log("Grammar loading failed with: " ++ e);
        None;
      };
    | None => None
    };
  };
};
