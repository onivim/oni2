/*
 * LanguageInfo.re
 */

open Oni_Core;
open Rench;

open Exthost.Extension;
open Scanner.ScanResult;

[@deriving show]
type t = {
  grammars: list(Contributions.Grammar.t),
  languages: list(Contributions.Language.t),
  extToLanguage: [@opaque] StringMap.t(string),
  languageToConfigurationPath: [@opaque] StringMap.t(string),
  languageToScope: [@opaque] StringMap.t(string),
  scopeToGrammarPath: [@opaque] StringMap.t(string),
  scopeToTreesitterPath: [@opaque] StringMap.t(option(string)),
};

let toString = languageInfo => {
  show(languageInfo)
  ++ "\n Grammars: \n"
  ++ StringMap.fold(
       (key, v, acc) => {acc ++ "\n" ++ "key: " ++ key ++ " val: " ++ v},
       languageInfo.scopeToGrammarPath,
       "",
     );
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

let _getLanguageTuples = (lang: Contributions.Language.t) => {
  List.map(extension => (extension, lang.id), lang.extensions);
};

let _getGrammars = (extensions: list(Scanner.ScanResult.t)) => {
  extensions |> List.map(v => v.manifest.contributes.grammars) |> List.flatten;
};

let _getLanguages = (extensions: list(Scanner.ScanResult.t)) => {
  extensions |> List.map(v => v.manifest.contributes.languages) |> List.flatten;
};

let ofExtensions = (extensions: list(Scanner.ScanResult.t)) => {
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
  open Contributions.Grammar;
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
         (prev, {id, configuration, _}: Contributions.Language.t) => {
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
