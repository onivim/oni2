[@deriving show({with_path: false})]
type t = {
  fontFamily: [@opaque] Revery.Font.Family.t,
  fontSize: float,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: [@opaque] Revery.Font.Smoothing.t,
  features: [@opaque] list(Revery.Font.Feature.t),
};

let default = {
  fontFamily: Revery.Font.Family.fromFile(Constants.defaultFontFile),
  fontSize: Constants.defaultFontSize,
  measuredWidth: 1.,
  measuredHeight: 1.,
  descenderHeight: 0.,
  smoothing: Revery.Font.Smoothing.default,
  features: [],
};
