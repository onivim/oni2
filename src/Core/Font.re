[@deriving show({with_path: false})]
type t = {
  fontFamily: [@opaque] Revery.Font.Family.t,
  fontSize: float,
  spaceWidth: float,
  underscoreWidth: float,
  avgCharWidth: float,
  maxCharWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: [@opaque] Revery.Font.Smoothing.t,
  features: [@opaque] list(Revery.Font.Feature.t),
  measurementCache: [@opaque] FontMeasurementCache.t,
};

let default = () => {
  let fontFamily = Revery.Font.Family.fromFile(Constants.defaultFontFile);
  let fontSize = Constants.defaultFontSize;
  let smoothing = Revery.Font.Smoothing.default;
  let features = [];
  {
    fontFamily,
    fontSize,
    spaceWidth: 8.4,
    underscoreWidth: 8.4,
    avgCharWidth: 34.37,
    maxCharWidth: 34.37,
    measuredHeight: 17.36,
    descenderHeight: 3.78,
    smoothing: Revery.Font.Smoothing.default,
    features,
    measurementCache:
      FontMeasurementCache.create(
        ~fontFamily,
        ~fontSize,
        ~smoothing,
        ~features,
      ),
  };
};

let measure = (uchar, {measurementCache, _}) =>
  FontMeasurementCache.measure(uchar, measurementCache);
