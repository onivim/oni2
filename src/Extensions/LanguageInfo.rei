/*
 * LanguageInfo.re
 */

open Oni_Core;
open Exthost.Extension;

type t;
let initial: t;

let defaultLanguage: string;

let getGrammars: t => list(Contributions.Grammar.t);

let getLanguageFromExtension: (t, string) => string;
let getLanguageFromFilePath: (t, string) => string;
let getLanguageFromBuffer: (t, Buffer.t) => string;

let getScopeFromLanguage: (t, string) => option(string);
let getScopeFromExtension: (t, string) => option(string);

let getLanguageConfigurationPath: (t, string) => option(string);

let getGrammarPathFromScope: (t, string) => option(string);
let getTreesitterPathFromScope: (t, string) => option(string);

let ofExtensions: list(Scanner.ScanResult.t) => t;
