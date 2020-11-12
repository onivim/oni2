type t = {
  family: Revery.Font.Family.t,
  size: float,
};

let default = {
  family:
    Revery.Font.Family.fromFiles((~weight, ~italic) =>
      switch (weight, italic) {
      | (Bold | UltraBold | Heavy, false) => "Inter-UI-Bold.ttf"
      | (Medium | SemiBold, false) => "Inter-UI-Medium.ttf"
      | (Medium | SemiBold | Bold | UltraBold | Heavy, true) => "Inter-UI-MediumItalic.ttf"
      | (Normal | Light | UltraLight | Thin | Undefined, true) => "Inter-UI-Italic.ttf"
      | (Normal | Light | UltraLight | Thin | Undefined, false) => "Inter-UI-Regular.ttf"
      }
    ),
  size: 12.,
};

let averageCharacterWidth = ({family, size}) => {
  family
  |> Revery.Font.Family.resolve(~italic=false, Revery.Font.Weight.Normal)
  |> Result.map((font: Revery.Font.t) => {
       let fontMetrics = Revery.Font.getMetrics(font, size);
       fontMetrics.avgCharWidth;
     })
  |> Result.value(~default=size);
};
