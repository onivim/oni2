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
