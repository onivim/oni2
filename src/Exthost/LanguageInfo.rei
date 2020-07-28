/*
 * LanguageInfo.re
 */
open Oni_Core;

open Exthost_Extension;

type t;
let initial: t;

let toString: t => string;

let defaultLanguage: string;

let getLanguageFromFilePath: (t, string) => string;
let getLanguageFromBuffer: (t, Buffer.t) => string;

let getScopeFromLanguage: (t, string) => option(string);
let getScopeFromFileName: (t, string) => option(string);
let getScopeFromBuffer: (t, Buffer.t) => option(string);

let getLanguageConfiguration: (t, string) => option(LanguageConfiguration.t);

let ofExtensions: list(Scanner.ScanResult.t) => t;
