/*
 * GrammarRepository.re
 */

open Oni_Core;

module Ext = Oni_Extensions;

type t = {
  isEmpty: bool,
  scopeToGrammar: Hashtbl.t(string, Textmate.Grammar.t),
  languageInfo: Ext.LanguageInfo.t,
  log: string => unit,
};

let create = (~isEmpty=false, ~log=(_)=>(), languageInfo) => {
  isEmpty, 
  log,
  scopeToGrammar: Hashtbl.create(32),
  languageInfo,
};

let empty = create(~isEmpty=true,Ext.LanguageInfo.empty);

let getGrammar = (~scope: string, gr: t) => {
    gr.log("getGrammar - scope: " ++ scope ++ " isEmpty: " ++ (gr.isEmpty ? "TRUE" : "false"));
    switch (Hashtbl.find_opt(gr.scopeToGrammar, scope)) {
    | Some(v) => 
      gr.log("getGrammar - using cached grammar.");
      Some(v);
    | None =>
      gr.log("getGrammar - querying language info");
      switch (Ext.LanguageInfo.getGrammarPathFromScope(gr.languageInfo, scope)) {
      | Some(grammarPath) =>
        gr.log("Loading grammar from: " ++ grammarPath);
        let json = Yojson.Safe.from_file(grammarPath);
        let resultGrammar = Textmate.Grammar.Json.of_yojson(json);

        switch (resultGrammar) {
        | Ok(grammar) =>
          gr.log("Grammar laoded successfully");
          Hashtbl.add(gr.scopeToGrammar, scope, grammar);
          Some(grammar);
        | Error(e) => 
          gr.log("Grammar loading failed with: " ++ e);
          None;
        };
      | None => None
      }
    };
};
