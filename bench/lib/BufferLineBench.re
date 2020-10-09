open EditorCoreTypes;
open Oni_Core;
open BenchFramework;

let largeBufferLine =
  String.make(10000, 'a') |> BufferLine.make(~measure=_ => 1.0);

let lengthSlow = () => {
  let _ = largeBufferLine |> BufferLine.lengthSlow;
  ();
};

let lengthBounded = () => {
  let _ =
    largeBufferLine
    |> BufferLine.lengthBounded(~max=5000 |> CharacterIndex.ofInt);
  ();
};

let getUcharExn = () => {
  let _ =
    largeBufferLine
    |> BufferLine.getUcharExn(~index=8000 |> CharacterIndex.ofInt);
  ();
};

let getPositionAndWidth = () => {
  let _ =
    largeBufferLine
    |> BufferLine.getUcharExn(~index=9000 |> CharacterIndex.ofInt);
  ();
};

let setup = () => ();
let options = Reperf.Options.create(~iterations=1000, ());

bench(~name="BufferLine: lengthSlow", ~options, ~setup, ~f=lengthSlow, ());

bench(
  ~name="BufferLine: lengthBounded",
  ~options,
  ~setup,
  ~f=lengthBounded,
  (),
);

bench(~name="BufferLine: getUcharExn", ~options, ~setup, ~f=getUcharExn, ());

bench(
  ~name="BufferLine: getPositionAndWidth",
  ~options,
  ~setup,
  ~f=getPositionAndWidth,
  (),
);
