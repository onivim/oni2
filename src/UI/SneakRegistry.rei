type callback = unit => unit;

let register: (ref(option(Revery.UI.node)), callback) => unit;
let unregister: ref(option(Revery.UI.node)) => unit;

let getSneaks: unit => list(Oni_Model.Sneak.sneakInfo);
