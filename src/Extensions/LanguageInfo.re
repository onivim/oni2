/*
 * LanguageInfo.re
 */

open Oni_Core;
open Rench;

module ExtensionScanner = Exthost.Extension.Scanner;
module ExtensionContributions = Exthost.Extension.Contributions;
module ExtensionScanResult = Exthost.Extension.Scanner.ScanResult;

open ExtensionScanResult;

type t = {
  grammars: list(ExtensionContributions.Grammar.t),
  languages: list(ExtensionContributions.Language.t),
  extToLanguage: StringMap.t(string),
  languageToConfigurationPath: StringMap.t(string),
  languageToScope: StringMap.t(string),
  scopeToGrammarPath: StringMap.t(string),
  scopeToTreesitterPath: StringMap.t(option(string)),
};

module Regexes = {
  let oniPath = Oniguruma.OnigRegExp.create("oni:\\/\\/([a-z]*)\\/(.*)");
};

let initial = {
  grammars: [],
  languages: [],
  extToLanguage: StringMap.empty,
  languageToConfigurationPath: StringMap.empty,
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
  let default = Path.extname(fp) |> getLanguageFromExtension(li);

  Regexes.oniPath
  |> Stdlib.Result.to_option
  |> Utility.OptionEx.flatMap(regex => {
       let matches = Oniguruma.OnigRegExp.search(fp, 0, regex);
       if (Array.length(matches) == 0) {
         None;
       } else {
         Some(Oniguruma.OnigRegExp.Match.getText(matches[1]));
       };
     })
  |> Stdlib.Option.value(~default);
};

let getLanguageFromBuffer = (li: t, buffer: Buffer.t) => {
  switch (Buffer.getFilePath(buffer)) {
  | None => defaultLanguage
  | Some(v) => getLanguageFromFilePath(li, v)
  };
};

let getLanguageConfigurationPath = (li: t, languageId: string) => {
  StringMap.find_opt(languageId, li.languageToConfigurationPath);
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
  li.scopeToTreesitterPath |> StringMap.find_opt(scope) |> Option.join;
};

let _getLanguageTuples = (lang: ExtensionContributions.Language.t) => {
  List.map(extension => (extension, lang.id), lang.extensions);
};

let _getGrammars = (extensions: list(ExtensionScanResult.t)) => {
  extensions |> List.map(v => v.manifest.contributes.grammars) |> List.flatten;
};

let _getLanguages = (extensions: list(ExtensionScanResult.t)) => {
  extensions |> List.map(v => v.manifest.contributes.languages) |> List.flatten;
};

let ofExtensions = (extensions: list(ExtensionScanResult.t)) => {
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

  let languageToConfigurationPath =
    languages
    |> List.fold_left(
         (prev, {id, configuration, _}: ExtensionContributions.Language.t) => {
           switch (configuration) {
           | None => prev
           | Some(configPath) => StringMap.add(id, configPath, prev)
           }
         },
         StringMap.empty,
       );

  {
    grammars,
    languages,
    extToLanguage,
    languageToConfigurationPath,
    languageToScope,
    scopeToGrammarPath,
    scopeToTreesitterPath,
  };
};
