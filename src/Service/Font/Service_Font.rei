open Oni_Core;

type font = {
  fontFamily: Revery.Font.Family.t,
  fontSize: float,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: Revery.Font.Smoothing.t,
  features: list(Revery.Font.Feature.t),
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
      ~fontLigatures: bool,
      ~fontSmoothing: ConfigurationValues.fontSmoothing
    ) =>
    Isolinear.Sub.t(msg);
};
