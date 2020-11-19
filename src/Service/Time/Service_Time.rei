module Sub: {
  let once:
    (
      ~uniqueId: string,
      ~delay: Revery.Time.t,
      ~msg: (~current: Revery.Time.t) => 'a
    ) =>
    Isolinear.Sub.t('a);
};
