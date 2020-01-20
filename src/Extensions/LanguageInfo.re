/*
 * LanguageInfo.re
 */

open Oni_Core;
open Oni_Core_Kernel;
open Rench;

open ExtensionScanner;

type t = {
  grammars: list(ExtensionContributions.Grammar.t),
  languages: list(ExtensionContributions.Language.t),
  extToLanguage: StringMap.t(string),
  languageToScope: StringMap.t(string),
  scopeToGrammarPath: StringMap.t(string),
  scopeToTreesitterPath: StringMap.t(option(string)),
};

let initial = {
  grammars: [],
  languages: [],
  extToLanguage: StringMap.empty,
  languageToScope: StringMap.empty,
  scopeToGrammarPath: StringMap.empty,
  scopeToTreesitterPath: StringMap.empty,
};

let getGrammars = (li: t) => {
  li.grammars;
};

let defaultLanguage = "plaintext";

let getLanguageFromExtension = (li: t, ext: string) => {
  switch (StringMap.find_opt(ext, li.extToLanguage)) {
  | Some(v) => v
  | None => defaultLanguage
  };
};

let getLanguageFromFilePath = (li: t, fp: string) => {
  Path.extname(fp) |> getLanguageFromExtension(li);
};

let getLanguageFromBuffer = (li: t, buffer: Buffer.t) => {
  switch (Buffer.getFilePath(buffer)) {
  | None => defaultLanguage
  | Some(v) => getLanguageFromFilePath(li, v)
  };
};

let getScopeFromLanguage = (li: t, languageId: string) => {
  StringMap.find_opt(languageId, li.languageToScope);
};

let getScopeFromExtension = (li: t, ext: string) => {
  getLanguageFromExtension(li, ext) |> getScopeFromLanguage(li);
};

let getGrammarPathFromScope = (li: t, scope: string) => {
  StringMap.find_opt(scope, li.scopeToGrammarPath);
};

let getTreesitterPathFromScope = (li: t, scope: string) => {
  li.scopeToTreesitterPath |> StringMap.find_opt(scope) |> Oni_Core_Utility.Option.join;
};

let _getLanguageTuples = (lang: ExtensionContributions.Language.t) => {
  List.map(extension => (extension, lang.id), lang.extensions);
};

let _getGrammars = (extensions: list(ExtensionScanner.t)) => {
  extensions |> List.map(v => v.manifest.contributes.grammars) |> List.flatten;
};

let _getLanguages = (extensions: list(ExtensionScanner.t)) => {
  extensions |> List.map(v => v.manifest.contributes.languages) |> List.flatten;
};

let ofExtensions = (extensions: list(ExtensionScanner.t)) => {
  let grammars = _getGrammars(extensions);
  let languages = _getLanguages(extensions);

  let extToLanguage =
    languages
    |> List.map(_getLanguageTuples)
    |> List.flatten
    |> List.fold_left(
         (prev, v) => {
           let (extension, language) = v;
           StringMap.add(extension, language, prev);
         },
         StringMap.empty,
       );
  open ExtensionContributions.Grammar;
  let languageToScope =
    grammars
    |> List.fold_left(
         (prev, curr) =>
           switch (curr.language) {
           | None => prev
           | Some(v) => StringMap.add(v, curr.scopeName, prev)
           },
         StringMap.empty,
       );

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

  {
    grammars,
    languages,
    extToLanguage,
    languageToScope,
    scopeToGrammarPath,
    scopeToTreesitterPath,
  };
};
