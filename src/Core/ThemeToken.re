type t = {
  index: int,
  backgroundColor: Revery.Color.t,
  foregroundColor: Revery.Color.t,
  syntaxScope: SyntaxScope.t,
  bold: bool,
  italic: bool,
};

let create =
    (
      ~index,
      ~backgroundColor,
      ~foregroundColor,
      ~syntaxScope,
      ~italic=false,
      ~bold=false,
      (),
    ) => {
  index,
  backgroundColor,
  foregroundColor,
  syntaxScope,
  bold,
  italic,
};

let toString =
    (
      {index, backgroundColor, foregroundColor, syntaxScope, bold, italic},
    ) => {
  Printf.sprintf(
    "ColorizedToken - index: %d foreground: %s background: %s scope: %s bold: %b italic: %b",
    index,
    backgroundColor |> Revery.Color.toString,
    foregroundColor |> Revery.Color.toString,
    syntaxScope |> SyntaxScope.toString,
    bold,
    italic,
  );
};
