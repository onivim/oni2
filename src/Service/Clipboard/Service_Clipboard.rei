module Effects: {
  let getClipboardText:
    (~toMsg: option(string) => 'a) => Isolinear.Effect.t('a);

  let setClipboardText: string => Isolinear.Effect.t(_);
};

module Testing: {
  let setClipboardProvider:
    (~get: unit => option(string), ~set: string => unit) => unit;
};
