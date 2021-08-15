open Oni_Core;

module Sub = {
  type findInFilesMsg =
    | GotMatches(list(Ripgrep.Match.t))
    | Completed
    | Error(string);

  type findInFilesParams = {
    exclude: list(string),
    include_: list(string),
    followSymlinks: bool,
    useIgnoreFiles: bool,
    directory: string,
    query: string,
    uniqueId: string,
    setup: Setup.t,
    enableRegex: bool,
    caseSensitive: bool,
  };

  module FindInFilesSub =
    Isolinear.Sub.Make({
      type nonrec msg = findInFilesMsg;

      type nonrec params = findInFilesParams;

      type state = {dispose: unit => unit};

      let name = "Service_Ripgrep.Sub.findInFiles";
      let id = ({uniqueId, directory, query, _}) =>
        Printf.sprintf("%s:%s:%s", uniqueId, directory, query);

      let init = (~params, ~dispatch) => {
        let ripgrep =
          Ripgrep.make(
            ~executablePath={
              params.setup.rgPath;
            },
          );
        let dispose =
          ripgrep.Ripgrep.findInFiles(
            ~followSymlinks=params.followSymlinks,
            ~useIgnoreFiles=params.useIgnoreFiles,
            ~searchExclude=params.exclude,
            ~searchInclude=params.include_,
            ~directory=params.directory,
            ~query=params.query,
            ~onUpdate=items => dispatch(GotMatches(items)),
            ~onComplete=() => {dispatch(Completed)},
            ~onError=msg => dispatch(Error(msg)),
            ~enableRegex=params.enableRegex,
            ~caseSensitive=params.caseSensitive,
            (),
          );
        {dispose: dispose};
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params as _, ~state) => {
        state.dispose();
      };
    });

  let findInFiles =
      (
        ~uniqueId: string,
        ~followSymlinks: bool,
        ~useIgnoreFiles: bool,
        ~exclude: list(string),
        ~include_: list(string),
        ~directory: string,
        ~query: string,
        ~setup: Setup.t,
        ~enableRegex=false,
        ~caseSensitive=false,
        toMsg,
      ) =>
    FindInFilesSub.create({
      followSymlinks,
      useIgnoreFiles,
      uniqueId,
      exclude,
      include_,
      directory,
      query,
      setup,
      enableRegex,
      caseSensitive,
    })
    |> Isolinear.Sub.map(toMsg);
};
