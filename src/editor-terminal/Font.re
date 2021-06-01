type t = {
  font: Revery.Font.t,
  fontSize: float,
  lineHeight: float,
  characterHeight: float,
  characterWidth: float,
  smoothing: Revery.Font.Smoothing.t,
};

let make =
    (
      ~smoothing=Revery.Font.Smoothing.default,
      ~size,
      ~lineHeight,
      font: Revery.Font.t,
    ) => {
  let fontSize = size;
  let {height, _}: Revery.Font.FontMetrics.t =
    Revery.Font.getMetrics(font, fontSize);
  let {width, _}: Revery.Font.measureResult =
    Revery.Font.measure(~smoothing, font, fontSize, "M");

  {
    font,
    fontSize,
    lineHeight,
    characterHeight: height,
    characterWidth: width,
    smoothing,
  };
};
