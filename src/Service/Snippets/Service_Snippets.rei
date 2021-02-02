module SnippetWithMetadata: {
  [@deriving show]
  type t = {
    snippet: string,
    prefix: string,
    description: string,
  };
};

module Sub: {
  let snippetFromFiles:
    (
      ~uniqueId: string,
      ~filePaths: list(Fp.t(Fp.absolute)),
      list(SnippetWithMetadata.t) => 'msg
    ) =>
    Isolinear.Sub.t('msg);
};
