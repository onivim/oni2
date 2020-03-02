open Oni_Core;
open BenchFramework;
open Feature_Editor;

// DATA

module Data = {
  let randomString = () =>
    String.init(Random.int(100), _ =>
      Char.chr(Char.code('a') + Random.int(29))
    );

  let randomBufferLine = () =>
    BufferLine.make(~indentation=IndentationSettings.default, randomString());

  let lines_10k_a = Array.init(10000, _ => randomString());
  let lines_10k_b = Array.init(10000, _ => randomString());
  let lines_100k = Array.init(100000, _ => randomString());

  let buffer_10k_nochanges =
    Buffer.ofLines(lines_10k_a) |> Buffer.setOriginalLines(lines_10k_a);

  let buffer_10k_randomchanges =
    Buffer.ofLines(lines_10k_a) |> Buffer.setOriginalLines(lines_10k_b);

  let buffer_10k_onelineoriginal =
    Buffer.ofLines(lines_10k_a)
    |> Buffer.setOriginalLines([|randomString()|]);

  let buffer_10k_onelinemodified =
    Buffer.ofLines([|randomString()|])
    |> Buffer.setOriginalLines(lines_10k_a);

  let buffer_100k_nochanges =
    Buffer.ofLines(lines_100k) |> Buffer.setOriginalLines(lines_100k);
};

// TESTS

module Tests = {
  let diff_10k_nochanges = () => {
    let _ = EditorDiffMarkers.generate(Data.buffer_10k_nochanges);
    ();
  };

  let diff_10k_randomchanges = () => {
    let _ = EditorDiffMarkers.generate(Data.buffer_10k_nochanges);
    ();
  };

  let diff_10k_onelineoriginal = () => {
    let _ = EditorDiffMarkers.generate(Data.buffer_10k_onelineoriginal);
    ();
  };

  let diff_10k_onelinemodified = () => {
    let _ = EditorDiffMarkers.generate(Data.buffer_10k_onelinemodified);
    ();
  };

  let diff_100k_nochanges = () => {
    let _ = EditorDiffMarkers.generate(Data.buffer_100k_nochanges);
    ();
  };
};

// PLUMBING

let setup = () => ();
let options = Reperf.Options.create(~iterations=100, ());

bench(
  ~name="EditorDiffMarkers: 10k lines, no changes",
  ~options,
  ~setup,
  ~f=Tests.diff_10k_nochanges,
  (),
);

bench(
  ~name="EditorDiffMarkers: 10k lines, random changes",
  ~options,
  ~setup,
  ~f=Tests.diff_10k_randomchanges,
  (),
);

bench(
  ~name="EditorDiffMarkers: 10k lines, 1 line original",
  ~options,
  ~setup,
  ~f=Tests.diff_10k_onelineoriginal,
  (),
);

bench(
  ~name="EditorDiffMarkers: 10k lines, 1 line modified",
  ~options,
  ~setup,
  ~f=Tests.diff_10k_onelinemodified,
  (),
);

let options = Reperf.Options.create(~iterations=10, ());

bench(
  ~name="EditorDiffMarkers: 100k lines, no changes",
  ~options,
  ~setup,
  ~f=Tests.diff_100k_nochanges,
  (),
);
