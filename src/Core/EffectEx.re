let value = (~name, v) =>
  Isolinear.Effect.createWithDispatch(~name, dispatch => dispatch(v));
