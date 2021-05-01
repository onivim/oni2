open Oni_Core;

module Sub = {
  type findInFilesMsg =
    | GotMatches(list(Ripgrep.Match.t))
    | Completed
    | Error(string);

  type findInFilesParams = {
    exclude: list(string),
    directory: string,
    query: string,
    uniqueId: string,
    ripgrep: Ripgrep.t,
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
        let dispose =
          params.ripgrep.Ripgrep.findInFiles(
            ~searchExclude=params.exclude,
            ~directory=params.directory,
            ~query=params.query,
            ~onUpdate=items => dispatch(GotMatches(items)),
            ~onComplete=() => {dispatch(Completed)},
            ~onError=msg => dispatch(Error(msg)),
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
        ~exclude: list(string),
        ~directory: string,
        ~query: string,
        ~ripgrep: Ripgrep.t,
        toMsg,
      ) => Isolinear.Sub.none;
};
