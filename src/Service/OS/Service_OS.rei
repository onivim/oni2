module Effect: {
  let openURL: string => Isolinear.Effect.t(_);
  let stat: (string, Unix.stats => 'msg) => Isolinear.Effect.t('msg);
};
