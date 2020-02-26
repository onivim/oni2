open Oni_Core;

type t = {
  fontFamily: string,
  fontSize: float,
  font: Revery.Font.t,
  measuredWidth: float,
  measuredHeight: float,
  descenderHeight: float,
  smoothing: Revery.Font.Smoothing.t,
};

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
