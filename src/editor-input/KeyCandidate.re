[@deriving show]
type t = list(KeyPress.t);

let ofKeyPress = keyPress => [keyPress];

let ofList = keyPresses => keyPresses;

let toList = keyPresses => keyPresses;

let exists = (~f, presses) => List.exists(f, presses);

let isModifier = exists(~f=key => KeyPress.isModifier(key));

let toString = keys =>
  keys |> List.map(KeyPress.toString) |> String.concat("\n");
