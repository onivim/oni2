
let rect = (x, y, width, height) => {
    Revery.Math.BoundingBox2d.create(
    x |> float, 
    y |> float, 
    width |> float, 
    height |> float)
}

let textArea: State.t => option(Revery.Math.BoundingBox2d.t) = (state) => {
    switch (FocusManager.current(state)) {
    | Editor => Some(rect(50, 50, 75, 75))

    | _ => Some(rect(100, 100, 100, 100))
    };
};
