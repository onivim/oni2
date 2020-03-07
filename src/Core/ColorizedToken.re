type t = {
  index: int,
  backgroundColor: Revery.Color.t,
  foregroundColor: Revery.Color.t,
  syntaxScope: SyntaxScope.t,
};

let create = (~index, ~backgroundColor, ~foregroundColor, ~syntaxScope, ()) => {
  index,
  backgroundColor,
  foregroundColor,
  syntaxScope,
};

let toString = ({index, backgroundColor, foregroundColor, syntaxScope}) => {
  Printf.sprintf(
    "ColorizedToken - index: %d foreground: %s background: %s scope: %s",
    index,
    backgroundColor |> Revery.Color.toString,
    foregroundColor |> Revery.Color.toString,
    syntaxScope |> SyntaxScope.toString,
  );
};
