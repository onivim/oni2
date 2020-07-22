/*
 * GrammarInfo.rei
 */
open Oni_Core;

open Exthost_Extension;

type t;

let initial: t;

let getGrammars: t => list(Contributions.Grammar.t);
let getGrammarPathFromScope: (t, string) => option(string);
let getTreesitterPathFromScope: (t, string) => option(string);

let ofExtensions: list(Scanner.ScanResult.t) => t;
