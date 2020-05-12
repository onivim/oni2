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

let empty = create(Ext.LanguageInfo.initial);

let getGrammar = (~scope: string, gr: t) => {
  gr.log("Trying to load grammar for scope: " ++ scope);
  switch (Hashtbl.find_opt(gr.scopeToGrammar, scope)) {
  | Some(v) =>
    gr.log("Got grammar for: " ++ scope);
    Some(v);
  | None =>
    switch (Ext.LanguageInfo.getGrammarPathFromScope(gr.languageInfo, scope)) {
    | Some(grammarPath) =>
      gr.log("Loading grammar from: " ++ grammarPath);

      switch (Yojson.Safe.from_file(grammarPath)) {
      | json =>
        switch (Textmate.Grammar.Json.of_yojson(json)) {
        | Ok(grammar) =>
          gr.log("JSON Grammar loaded successfully");
          Hashtbl.add(gr.scopeToGrammar, scope, grammar);
          Some(grammar);

        | Error(e) =>
          gr.log("Grammar loading failed with: " ++ e);
          None;
        }

      | exception (Yojson.Json_error(_)) =>
        switch (Textmate.Grammar.Xml.of_file(grammarPath)) {
        | Ok(grammar) =>
          gr.log("XML Grammar loaded successfully");
          Hashtbl.add(gr.scopeToGrammar, scope, grammar);
          Some(grammar);

        | Error(e) =>
          gr.log("Grammar loading failed with: " ++ e);
          None;
        }
      };

    | None =>
      gr.log("Unable to find grammar for: " ++ scope);
      None;
    }
  };
};
