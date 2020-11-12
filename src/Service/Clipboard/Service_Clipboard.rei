module Effects: {
  let getClipboardText:
    (~toMsg: option(string) => 'a) => Isolinear.Effect.t('a);
};

module Testing: {
  let setClipboardProvider: (~get: unit => option(string)) => unit;
};
