module Sub: {
  let snippetFromFiles:
    (
      ~filePaths: list(Fp.t(Fp.absolute)),
      result(list(Snippet.t), string) => 'msg
    ) =>
    Isolinear.Sub.t('msg);
};
