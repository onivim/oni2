
let of_ = (~name, v) => Isolinear.Effect.createWithDispatch(
    ~name,
    dispatch => dispatch(v));
