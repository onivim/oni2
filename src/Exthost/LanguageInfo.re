/*
 * LanguageInfo.re
 */

open Oni_Core;
open Oni_Core.Utility;

open Exthost_Extension;
open Scanner.ScanResult;

module Log = (val Log.withNamespace("Oni2.LanguageInfo"));

type configurationLoadState =
  | Loaded(LanguageConfiguration.t)
  | ErrorLoading;

type patternLanguagePair = {
  pattern: Oniguruma.OnigRegExp.t,
  language: string,
};

[@deriving show]
type t = {
  languages: list(Contributions.Language.t),
  extToLanguage: [@opaque] StringMap.t(string),
  fileNameToLanguage: [@opaque] StringMap.t(string),
  fileNamePatternToLanguage: [@opaque] list(patternLanguagePair),
  firstLineToLanguage: [@opaque] list(patternLanguagePair),
  languageCache: [@opaque] Hashtbl.t(string, configurationLoadState),
  languageToConfigurationPath: [@opaque] StringMap.t(string),
  languageToScope: [@opaque] StringMap.t(string),
};

let defaultExtensionTypes =
  [
    (".png", "image"),
    (".gif", "image"),
    (".tga", "image"),
    (".jpg", "image"),
    (".jpeg", "image"),
    (".bmp", "image"),
  ]
  |> List.to_seq
  |> StringMap.of_seq;

let languages = ({languages, _}) => {
  languages
  |> List.fold_left(
       (acc, {id, _}: Contributions.Language.t) => {
         acc |> StringMap.add(id, ())
       },
       StringMap.empty,
     )
  |> StringMap.bindings
  |> List.map(fst);
};

let toString = languageInfo => {
  show(languageInfo);
};

module Regexes = {
  let oniPath = Oniguruma.OnigRegExp.create("oni:\\/\\/([a-z]*)\\/(.*)");
};

let initial = {
  languages: [],
  languageCache: Hashtbl.create(16),
  extToLanguage: StringMap.empty,
  fileNameToLanguage: StringMap.empty,
  fileNamePatternToLanguage: [],
  firstLineToLanguage: [],
  languageToConfigurationPath: StringMap.empty,
  languageToScope: StringMap.empty,
};

let defaultLanguage = Oni_Core.Buffer.FileType.default;

let getLanguageFromExtension = (li: t, ext: string) => {
  li.extToLanguage
  |> StringMap.find_opt(ext)
  |> OptionEx.or_lazy(() => StringMap.find_opt(ext, defaultExtensionTypes))
  |> Option.value(~default=defaultLanguage);
};

let getLanguageFromFileNamePattern = (li: t, fileName: string) =>
  switch (
    List.find_opt(
      p => Oniguruma.OnigRegExp.test(fileName, p.pattern),
      li.fileNamePatternToLanguage,
    )
  ) {
  | Some(l) => l.language
  | None => defaultLanguage
  };

let getLanguageFromFileName = (li: t, fileName: string) => {
  let lowercaseFileName = String.lowercase_ascii(fileName);

  let fileNameMatch =
    StringMap.find_opt(lowercaseFileName, li.fileNameToLanguage);

  switch (fileNameMatch) {
  | Some(v) => v
  | None => getLanguageFromFileNamePattern(li, fileName)
  };
};

let getLanguageFromOniPath = (fp: string) => {
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
  |> Stdlib.Option.value(~default=defaultLanguage);
};

let getLanguageFromFirstLine = (li: t, buffer: Buffer.t) => {
  let firstLine =
    if (Buffer.getNumberOfLines(buffer) > 0) {
      Buffer.getLine(0, buffer) |> BufferLine.raw;
    } else {
      "";
    };

  if (firstLine == "") {
    defaultLanguage;
  } else {
    switch (
      List.find_opt(
        p => Oniguruma.OnigRegExp.test(firstLine, p.pattern),
        li.firstLineToLanguage,
      )
    ) {
    | Some(l) => l.language
    | None => defaultLanguage
    };
  };
};

let getLanguageFromFilePath = (li: t, fp: string) => {
  let fileName = Utility.Path.filename(fp);
  let extension = Utility.Path.getExtension(fp);

  let updateIfDefault = (f, res) =>
    if (res == defaultLanguage) {
      f();
    } else {
      res;
    };

  getLanguageFromOniPath(fp)
  |> updateIfDefault(() => getLanguageFromFileName(li, fileName))
  |> updateIfDefault(() => getLanguageFromExtension(li, extension));
};

let getLanguageFromBuffer = (li: t, buffer: Buffer.t) => {
  let filePath =
    switch (Buffer.getFilePath(buffer)) {
    | None => ""
    | Some(v) => v
    };

  let updateIfDefault = (f, res) =>
    if (res == defaultLanguage) {
      f();
    } else {
      res;
    };

  getLanguageFromFilePath(li, filePath)
  |> updateIfDefault(() => getLanguageFromFirstLine(li, buffer));
};

module Internal = {
  let getLanguageConfigurationPath = (li: t, languageId: string) => {
    StringMap.find_opt(languageId, li.languageToConfigurationPath);
  };

  let loadLanguage: string => result(LanguageConfiguration.t, string) =
    path => {
      path
      |> JsonEx.from_file
      |> ResultEx.flatMap(json =>
           json
           |> Json.Decode.decode_value(LanguageConfiguration.decode)
           |> Stdlib.Result.map_error(Json.Decode.string_of_error)
         )
      |> Stdlib.Result.map_error(FunEx.tap(Log.error));
    };

  let loadLanguageConfiguration = (~languageId, languageInfo: t) => {
    languageId
    |> getLanguageConfigurationPath(languageInfo)
    |> Option.map(
         FunEx.tap(p =>
           Log.infof(m =>
             m("Got path for %s, loading from %s", languageId, p)
           )
         ),
       )
    |> OptionEx.flatMap(path => {
         switch (loadLanguage(path)) {
         | Ok(languageConfig) =>
           Hashtbl.add(
             languageInfo.languageCache,
             languageId,
             Loaded(languageConfig),
           );
           Some(languageConfig);
         | Error(_) =>
           Hashtbl.add(languageInfo.languageCache, languageId, ErrorLoading);
           None;
         }
       });
  };
};

let getLanguageConfiguration = (languageInfo: t, languageId: string) => {
  languageId
  |> Hashtbl.find_opt(languageInfo.languageCache)
  |> (
    fun
    | Some(Loaded(languageConfig)) => Some(languageConfig)
    | Some(ErrorLoading) => None
    | None => Internal.loadLanguageConfiguration(~languageId, languageInfo)
  );
};

let getScopeFromLanguage = (li: t, languageId: string) => {
  StringMap.find_opt(languageId, li.languageToScope);
};

let getScopeFromFileName = (li: t, fileName: string) => {
  getLanguageFromFileName(li, fileName) |> getScopeFromLanguage(li);
};

let getScopeFromBuffer = (li: t, buffer: Oni_Core.Buffer.t) => {
  getLanguageFromBuffer(li, buffer) |> getScopeFromLanguage(li);
};

let getExtensionLanguageTuples = (lang: Contributions.Language.t) => {
  List.map(extension => (extension, lang.id), lang.extensions);
};

let getFileNameLanguageTuples = (lang: Contributions.Language.t) => {
  List.map(
    fileName => (String.lowercase_ascii(fileName), lang.id),
    lang.filenames,
  );
};

let getFilenamePatternTuples = (lang: Contributions.Language.t) => {
  List.map(pattern => (pattern, lang.id), lang.filenamePatterns);
};

let getListOfGrammars = (extensions: list(Scanner.ScanResult.t)) => {
  extensions |> List.map(v => v.manifest.contributes.grammars) |> List.flatten;
};

let getListOfLanguages = (extensions: list(Scanner.ScanResult.t)) => {
  extensions |> List.map(v => v.manifest.contributes.languages) |> List.flatten;
};

let addExtensions = (extensions: list(Scanner.ScanResult.t), model: t) => {
  let grammars = getListOfGrammars(extensions);
  let languages = getListOfLanguages(extensions);

  let extToLanguage =
    languages
    |> List.map(getExtensionLanguageTuples)
    |> List.flatten
    |> List.fold_left(
         (prev, v) => {
           let (extension, language) = v;
           StringMap.add(extension, language, prev);
         },
         model.extToLanguage,
       );

  let fileNameToLanguage =
    languages
    |> List.map(getFileNameLanguageTuples)
    |> List.flatten
    |> List.fold_left(
         (prev, v) => {
           let (fileName, language) = v;
           StringMap.add(fileName, language, prev);
         },
         model.fileNameToLanguage,
       );

  let fileNamePatternToLanguage =
    languages
    |> List.map(getFilenamePatternTuples)
    |> List.flatten
    |> List.fold_left(
         (prev, v) => {
           let (regex, language) = v;
           switch (Oniguruma.OnigRegExp.create(regex)) {
           | Error(_) => prev
           | Ok(pattern) => [{pattern, language}, ...prev]
           };
         },
         model.fileNamePatternToLanguage,
       );

  let firstLineToLanguage =
    languages
    |> List.fold_left(
         (prev, lang) => {
           Contributions.Language.(
             switch (lang.firstLine) {
             | Some(regex) =>
               let language = lang.id;
               switch (Oniguruma.OnigRegExp.create(regex)) {
               | Error(_) => prev
               | Ok(pattern) => [{pattern, language}, ...prev]
               };
             | None => prev
             }
           )
         },
         model.firstLineToLanguage,
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
         model.languageToScope,
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
         model.languageToConfigurationPath,
       );

  {
    ...model,
    languages,
    extToLanguage,
    fileNameToLanguage,
    fileNamePatternToLanguage,
    firstLineToLanguage,
    languageToConfigurationPath,
    languageToScope,
  };
};
