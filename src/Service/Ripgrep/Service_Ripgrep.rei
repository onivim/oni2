open Oni_Core;

module Sub: {
  type findInFilesMsg =
    | GotMatches(list(Ripgrep.Match.t))
    | Completed
    | Error(string);

  let findInFiles:
    (
      ~uniqueId: string,
      ~followSymlinks: bool,
      ~useIgnoreFiles: bool,
      ~exclude: list(string),
      ~include_: list(string),
      ~directory: string,
      ~query: string,
      ~setup: Setup.t,
      ~enableRegex: bool=?,
      ~caseSensitive: bool=?,
      findInFilesMsg => 'msg
    ) =>
    Isolinear.Sub.t('msg);
};
