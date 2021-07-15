module BoundingBox2d = Revery.Math.BoundingBox2d;
let rect = (x, y, width, height) => {
  Revery.Math.BoundingBox2d.create(
    x |> float,
    y |> float,
    width |> float,
    height |> float,
  );
};

module MutableState = {
  let current: ref(option(BoundingBox2d.t)) = ref(None);
};

let reset = () => MutableState.current := None;

let set = maybeBbox => MutableState.current := maybeBbox;

let textArea = _state => {
  MutableState.current
    // switch (FocusManager.current(state)) {
    // | Editor => Some(rect(50, 50, 75, 75))
    // | Extensions => None
    // | _ => Some(rect(100, 100, 100, 100))
    ^;
    // };
};
