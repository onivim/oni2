module Effects: {
  let getClipboardText:
    (~toMsg: option(string) => 'a) => Isolinear.Effect.t('a);
};
