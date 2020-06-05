type t = {
  normal: Revery.Font.Family.t,
  italic: Revery.Font.Family.t,
  bold: Revery.Font.Family.t,
  semiBold: Revery.Font.Family.t,
  semiBoldItalic: Revery.Font.Family.t,
  size: float,
};

let default = {
  normal: Revery.Font.Family.fromFile("Inter-UI-Regular.ttf"),
  bold: Revery.Font.Family.fromFile("Inter-UI-Bold.ttf"),
  italic: Revery.Font.Family.fromFile("Inter-UI-Italic.ttf"),
  semiBold: Revery.Font.Family.fromFile("Inter-UI-Medium.ttf"),
  semiBoldItalic: Revery.Font.Family.fromFile("Inter-UI-MediumItalic.ttf"),
  size: 12.,
};
