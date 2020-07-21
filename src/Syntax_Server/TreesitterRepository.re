/*
 * TreesitterRepository.re
 */

open Oni_Syntax;

type t = {
  scopeToConverter: Hashtbl.t(string, TreeSitterScopes.TextMateConverter.t),
  languageInfo: Exthost.LanguageInfo.t,
  log: string => unit,
};

let create = (~log=_ => (), languageInfo) => {
  log,
  scopeToConverter: Hashtbl.create(32),
  languageInfo,
};

let empty = create(Exthost.LanguageInfo.initial);

let getScopeConverter = (~scope: string, gr: t) => {
  switch (Hashtbl.find_opt(gr.scopeToConverter, scope)) {
  | Some(v) =>
    gr.log("getScopeConverter - using cached grammar.");
    Some(v);
  | None =>
    gr.log(
      "getScopeConverter - querying language info for language: " ++ scope,
    );
    switch (
      Exthost.LanguageInfo.getTreesitterPathFromScope(gr.languageInfo, scope)
    ) {
    | Some(grammarPath) =>
      gr.log("Loading tree sitter converter from: " ++ grammarPath);
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
