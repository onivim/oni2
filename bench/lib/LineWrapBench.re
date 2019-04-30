open Oni_Core;
open BenchFramework;

let setup = () => ();

let longLine = String.make(100000, 'a');

let wrapLine = (line, wrapPoint, ()) => {
  LineWrap.create(line, wrapPoint) |> ignore;
};

let options = Reperf.Options.create(~iterations=1000, ());

bench(
  ~name="LineWrap: Apply wrapping to long line (100000 characters)",
  ~setup,
  ~options,
  ~f=wrapLine(longLine, 50),
  (),
);
