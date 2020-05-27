// EFFECTS
module Effects: {
  module SCM: {
    let provideOriginalResource:
      (
        ~handles: list(int),
        Exthost.Client.t,
        string,
        Oni_Core.Uri.t => 'msg
      ) =>
      Isolinear.Effect.t('msg);

    let onInputBoxValueChange:
      (~handle: int, ~value: string, Exthost.Client.t) =>
      Isolinear.Effect.t(_);
  };
};

module Sub: {
  let buffer:
    (~buffer: Oni_Core.Buffer.t, ~client: Exthost.Client.t) =>
    Isolinear.Sub.t(unit);
};
