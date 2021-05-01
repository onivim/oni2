open Oni_Core;

module Sub: {
  type ripgrepMsg =
    | GotMatches(list(Ripgrep.Match.t))
    | Completed
    | Error(string);

  let ripgrep:
    (
      ~exclude: list(string),
      ~directory: string,
      ~query: string,
      ~ripgrep: Ripgrep.t,
      ripgrepMsg => 'msg
    ) =>
    Isolinear.Sub.t('msg);
};
