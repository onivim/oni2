// type t = {
//     //maybeStaticKeybindings: option(list(Schema.keybinding))
// };

type t = unit;

let static = _keybindings => ();

let file = _path => ();

let notifyFileSaved = (_path, loader) => loader;

let sub = loader => Isolinear.Sub.none;
