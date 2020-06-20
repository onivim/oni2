type t = {
  family: Revery.Font.Family.t,
  size: float,
};

let default = {
  family:
    Revery.Font.Family.fromFiles((~weight, ~italic, ~mono as _) =>
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
