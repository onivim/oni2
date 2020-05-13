/*
 * TreesitterRepository.re
 */

open Oni_Core;
open Oni_Syntax;

module Ext = Oni_Extensions;

module Log = (val Log.withNamespace("TreeSitterRepository"));

type t = {
  scopeToConverter: Hashtbl.t(string, TreeSitterScopes.TextMateConverter.t),
  languageInfo: Ext.LanguageInfo.t,
  log: string => unit,
};

let create = (~log=_ => (), languageInfo) => {
  log,
  scopeToConverter: Hashtbl.create(32),
  languageInfo,
};

let empty = create(Ext.LanguageInfo.initial);

let getScopeConverter = (~scope: string, gr: t) => {
  switch (Hashtbl.find_opt(gr.scopeToConverter, scope)) {
  | Some(v) =>
    Log.tracef( m => m("using cached grammar for scope: %s", scope));
    Some(v);
  | None =>
    Log.tracef(m => m(
      "getScopeConverter - querying language info for language: %s", scope,
    ));
    switch (
      Ext.LanguageInfo.getTreesitterPathFromScope(gr.languageInfo, scope)
    ) {
    | Some(grammarPath) =>
      Log.infof(m => m(
      "Loading tree sitter converter from: %s", grammarPath
      ));
      let json =
        Yojson.Safe.from_file(grammarPath)
        |> Yojson.Safe.Util.member("scopes");
      let grammar = TreeSitterScopes.TextMateConverter.of_yojson(json);
      Hashtbl.add(gr.scopeToConverter, scope, grammar);
      Some(grammar);
    | None => None
    };
  };
};
