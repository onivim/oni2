open Oni_Core;
open BenchFramework;

let largeBufferLine =
  String.make(10000, 'a')
  |> BufferLine.make(~indentation=IndentationSettings.default);

let slowLengthUtf8 = () => {
  let _ = largeBufferLine |> BufferLine.slowLengthUtf8;
  ();
};

let boundedLengthUtf8 = () => {
  let _ = largeBufferLine |> BufferLine.boundedLengthUtf8(~max=5000);
  ();
};

let unsafeGetUChar = () => {
  let _ = largeBufferLine |> BufferLine.unsafeGetUChar(~index=8000);
  ();
};

let getPositionAndWidth = () => {
  let _ = largeBufferLine |> BufferLine.unsafeGetUChar(~index=9000);
  ();
};

let setup = () => ();
let options = Reperf.Options.create(~iterations=1000, ());

bench(
  ~name="BufferLine: slowLengthUtf8",
  ~options,
  ~setup,
  ~f=slowLengthUtf8,
  (),
);

bench(
  ~name="BufferLine: boundedLengthUtf8",
  ~options,
  ~setup,
  ~f=boundedLengthUtf8,
  (),
);

bench(
  ~name="BufferLine: unsafeGetUChar",
  ~options,
  ~setup,
  ~f=unsafeGetUChar,
  (),
);

bench(
  ~name="BufferLine: getPositionAndWidth",
  ~options,
  ~setup,
  ~f=getPositionAndWidth,
  (),
);
