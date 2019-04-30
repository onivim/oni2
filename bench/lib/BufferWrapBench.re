open Oni_Model;
open BenchFramework;

open Revery.UI;

let rootNode = (new node)();

let setup = () => ();

let mediumBuffer =
  Buffer.ofLines(Array.make(10000, "This buffer is pretty big and has a lot of text in its line which may be an issue for word wrapping but we'll see what the benchmark says"));

let wrapBuffer = (buffer) => () => {
	BufferWrap.create(buffer, 25) |> ignore;
};

let options = Reperf.Options.create(~iterations=1000, ());

bench(
  ~name="BufferWrap: Apply wrapping to buffer (10000 Lines)",
  ~setup,
  ~options,
  ~f=wrapBuffer(mediumBuffer),
  (),
);
