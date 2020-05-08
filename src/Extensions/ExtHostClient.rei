module Core = Oni_Core;

// SCM

module SCM: {
  // EFFECTS
  module Effects: {
    let provideOriginalResource:
      (~handles: list(int), Exthost.Client.t, string, Core.Uri.t => 'msg) =>
      Isolinear.Effect.t('msg);

    let onInputBoxValueChange:
      (~handle: int, ~value: string, Exthost.Client.t) => Isolinear.Effect.t(_);
  };
};
