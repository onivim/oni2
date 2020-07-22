/*
 * TreesitterRepository.re
 */

open Oni_Syntax;

type t = {
  scopeToConverter: Hashtbl.t(string, TreeSitterScopes.TextMateConverter.t),
  grammarInfo: Exthost.GrammarInfo.t,
  log: string => unit,
};

let create = (~log=_ => (), grammarInfo) => {
  log,
  scopeToConverter: Hashtbl.create(32),
  grammarInfo,
};

let empty = create(Exthost.GrammarInfo.initial);

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
      Exthost.GrammarInfo.getTreesitterPathFromScope(gr.grammarInfo, scope)
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
