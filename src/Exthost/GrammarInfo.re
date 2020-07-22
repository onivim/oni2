/*
 * GrammarInfo.re
 */

open Oni_Core;

open Exthost_Extension;
open Scanner.ScanResult;

module Log = (val Log.withNamespace("Oni2.GrammarInfo"));

[@deriving show]
type t = {
  grammars: list(Contributions.Grammar.t),
  scopeToGrammarPath: [@opaque] StringMap.t(string),
  scopeToTreesitterPath: [@opaque] StringMap.t(option(string)),
};

let toString = grammarInfo => {
  show(grammarInfo)
  ++ "\n Grammars: \n"
  ++ StringMap.fold(
       (key, v, acc) => {acc ++ "\n" ++ "key: " ++ key ++ " val: " ++ v},
       grammarInfo.scopeToGrammarPath,
       "",
     );
};

let initial = {
  grammars: [],
  scopeToGrammarPath: StringMap.empty,
  scopeToTreesitterPath: StringMap.empty,
};

let getGrammars = (li: t) => {
  li.grammars;
};
let getGrammarPathFromScope = (li: t, scope: string) => {
  StringMap.find_opt(scope, li.scopeToGrammarPath);
};

let getTreesitterPathFromScope = (li: t, scope: string) => {
  li.scopeToTreesitterPath |> StringMap.find_opt(scope) |> Option.join;
};

module Internal = {
  let getGrammars = (extensions: list(Scanner.ScanResult.t)) => {
    extensions
    |> List.map(v => v.manifest.contributes.grammars)
    |> List.flatten;
  };
};

let ofExtensions = (extensions: list(Scanner.ScanResult.t)) => {
  let grammars = Internal.getGrammars(extensions);
  open Contributions.Grammar;

  let scopeToGrammarPath =
    grammars
    |> List.fold_left(
         (prev, curr) => {StringMap.add(curr.scopeName, curr.path, prev)},
         StringMap.empty,
       );

  let scopeToTreesitterPath =
    grammars
    |> List.fold_left(
         (prev, curr) => {
           StringMap.add(curr.scopeName, curr.treeSitterPath, prev)
         },
         StringMap.empty,
       );

  {grammars, scopeToGrammarPath, scopeToTreesitterPath};
};
