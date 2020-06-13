type t = {
  index: int,
  backgroundColor: Revery.Color.t,
  foregroundColor: Revery.Color.t,
  syntaxScope: SyntaxScope.t,
  bold: bool,
  italicized: bool,
};

let create =
    (
      ~index,
      ~backgroundColor,
      ~foregroundColor,
      ~syntaxScope,
      ~italicized=false,
      ~bold=false,
      (),
    ) => {
  index,
  backgroundColor,
  foregroundColor,
  syntaxScope,
  bold,
  italicized,
};

let toString =
    (
      {index, backgroundColor, foregroundColor, syntaxScope, bold, italicized},
    ) => {
  Printf.sprintf(
    "ColorizedToken - index: %d foreground: %s background: %s scope: %s bold: %b italicized: %b",
    index,
    backgroundColor |> Revery.Color.toString,
    foregroundColor |> Revery.Color.toString,
    syntaxScope |> SyntaxScope.toString,
    bold,
    italicized,
  );
};
