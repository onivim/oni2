// TODO:
// 1) Load snippets from path (implement snippetFromFiles)
// 2) Add placeholder text for snippet file, chain up open
// 3) Expand `snippetFromFiles` to include user snippets, too
open Oni_Core;

module SnippetWithMetadata: {
  [@deriving show]
  type t = {
    snippet: string,
    prefix: string,
    description: string,
    scopes: list(string),
  };
};

module SnippetFileMetadata: {
  [@deriving show]
  type t = {
    language: option(string),
    filePath: FpExp.t(FpExp.absolute),
    isCreated: bool,
  };
};

module Effect: {
  let createSnippetFile:
    (
      ~filePath: FpExp.t(FpExp.absolute),
      result(FpExp.t(FpExp.absolute), string) => 'msg
    ) =>
    Isolinear.Effect.t('msg);

  let clearCachedSnippets:
    (~filePath: FpExp.t(FpExp.absolute)) => Isolinear.Effect.t(_);

  let snippetFromFiles:
    (
      ~fileType: string,
      ~filePaths: list(FpExp.t(FpExp.absolute)),
      list(SnippetWithMetadata.t) => 'msg
    ) =>
    Isolinear.Effect.t('msg);

  let getUserSnippetFiles:
    (
      ~languageInfo: Exthost.LanguageInfo.t,
      list(SnippetFileMetadata.t) => 'msg
    ) =>
    Isolinear.Effect.t('msg);
};

module Sub: {
  let snippetFromFiles:
    (
      ~uniqueId: string,
      ~fileType: string,
      ~filePaths: list(FpExp.t(FpExp.absolute)),
      list(SnippetWithMetadata.t) => 'msg
    ) =>
    Isolinear.Sub.t('msg);
};
