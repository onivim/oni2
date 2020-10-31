module Sub: {
  let once:
    (~uniqueId: string, ~delay: Revery.Time.t, ~msg: 'a) =>
    Isolinear.Sub.t('a);
};
