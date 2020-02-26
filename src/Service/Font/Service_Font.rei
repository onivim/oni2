open Oni_Core;

type t = {
  fontFile: string,
  fontSize: float,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: Revery.Font.Smoothing.t,
};

let default: t;

let measure: (~text: string, t) => float;

let getHeight: t => float;

[@deriving show({with_path: false})]
type msg =
  | FontLoaded(t)
  | FontLoadError(string);

module Sub: {
  let font:
    (
      ~fontFamily: string,
      ~fontSize: float,
      ~fontSmoothing: ConfigurationValues.fontSmoothing
    ) =>
    Isolinear.Sub.t(msg);
};
