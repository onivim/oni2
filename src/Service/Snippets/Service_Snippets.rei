module SnippetWithMetadata: {
  type t = {
    snippet: Snippet.t,
    prefix: string,
    description: string,
  };
};

module Sub: {
  let snippetFromFiles:
    (
      ~filePaths: list(Fp.t(Fp.absolute)),
      result(list(SnippetWithMetadata.t), string) => 'msg
    ) =>
    Isolinear.Sub.t('msg);
};
