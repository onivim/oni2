// TODO:
// 1) Load snippets from path (implement snippetFromFiles)
// 2) Add placeholder text for snippet file, chain up open
// 3) Expand `snippetFromFiles` to include user snippets, too

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
    filePath: Fp.t(Fp.absolute),
    isCreated: bool,
  };
};

module Effect: {
  let createSnippetFile:
    (
      ~filePath: Fp.t(Fp.absolute),
      result(Fp.t(Fp.absolute), string) => 'msg
    ) =>
    Isolinear.Effect.t('msg);

  let clearCachedSnippets:
    (~filePath: Fp.t(Fp.absolute)) => Isolinear.Effect.t(_);

  let snippetFromFiles:
    (
      ~fileType: string,
      ~filePaths: list(Fp.t(Fp.absolute)),
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
      ~filePaths: list(Fp.t(Fp.absolute)),
      list(SnippetWithMetadata.t) => 'msg
    ) =>
    Isolinear.Sub.t('msg);
};
