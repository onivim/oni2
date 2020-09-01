open Oni_Core;

[@deriving show({with_path: false})]
type font =
  Oni_Core.Font.t = {
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
  };

let toString: font => string;

let default: font;

let resolveWithFallback:
  (~italic: bool=?, Revery_Font.Weight.t, Revery_Font.Family.t) =>
  Revery_Font.t;

let measure: (~text: string, font) => float;

let getHeight: font => float;

[@deriving show({with_path: false})]
type msg =
  | FontLoaded(font)
  | FontLoadError(string);

module Sub: {
  let font:
    (
      ~uniqueId: string,
      ~fontFamily: string,
      ~fontSize: float,
      ~fontLigatures: ConfigurationValues.fontLigatures,
      ~fontSmoothing: ConfigurationValues.fontSmoothing
    ) =>
    Isolinear.Sub.t(msg);
};
