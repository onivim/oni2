/*
 * LanguageInfo.re
 */

open Oni_Core;
open Oni_Extensions;
open Rench;

open Oni_Extensions.ExtensionScanner;

type t = {
  grammars: list(ExtensionContributions.Grammar.t),
  languages: list(ExtensionContributions.Language.t),
  extToLanguage: StringMap.t(string),
  languageToScope: StringMap.t(string),
};

let getGrammars = (li: t) => {
  li.grammars;
};

let getLanguageFromExtension = (li: t, ext: string) => {
  switch (StringMap.find_opt(ext, li.extToLanguage)) {
  | Some(v) => v
  | None => "plaintext"
  };
};

let getLanguageFromFilePath = (li: t, ext: string) => {
  Path.extname(ext) |> getLanguageFromExtension(li);
};

let getScopeFromLanguage = (li: t, languageId: string) => {
  StringMap.find_opt(languageId, li.languageToScope);
};

let getScopeFromExtension = (li: t, ext: string) => {
  getLanguageFromExtension(li, ext) |> getScopeFromLanguage(li);
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

let create = () => {
  grammars: [],
  languages: [],
  extToLanguage: StringMap.empty,
  languageToScope: StringMap.empty,
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

  {grammars, languages, extToLanguage, languageToScope};
};
