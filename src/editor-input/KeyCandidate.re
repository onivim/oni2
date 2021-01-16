[@deriving show]
type t = list(KeyPress.t);

let ofKeyPress = keyPress => [keyPress];

let ofList = keyPresses => keyPresses;

let toList = keyPresses => keyPresses;

let exists = (~f, presses) => List.exists(f, presses);
