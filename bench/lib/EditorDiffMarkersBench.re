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
    BufferLine.make(~measure=_ => 1.0, randomString());

  let lines_10k_a = Array.init(10000, _ => randomString());
  let lines_10k_b = Array.init(10000, _ => randomString());
  let lines_100k = Array.init(100000, _ => randomString());

  let buffer_10k_nochanges =
    Buffer.ofLines(~font=Font.default(), lines_10k_a);
  let scm_10k_nochanges =
    Feature_SCM.initial
    |> Feature_SCM.setOriginalLines(buffer_10k_nochanges, lines_10k_a);

  let buffer_10k_randomchanges =
    Buffer.ofLines(~font=Font.default(), lines_10k_a);

  let scm_10k_randomchanges =
    Feature_SCM.initial
    |> Feature_SCM.setOriginalLines(buffer_10k_randomchanges, lines_10k_b);

  let buffer_10k_onelineoriginal =
    Buffer.ofLines(~font=Font.default(), lines_10k_a);

  let scm_10k_onelineoriginal =
    Feature_SCM.initial
    |> Feature_SCM.setOriginalLines(
         buffer_10k_onelineoriginal,
         [|randomString()|],
       );

  let buffer_10k_onelinemodified =
    Buffer.ofLines(~font=Font.default(), [|randomString()|]);

  let scm_10k_onelinemodified =
    Feature_SCM.initial
    |> Feature_SCM.setOriginalLines(buffer_10k_onelinemodified, lines_10k_a);

  let buffer_100k_nochanges =
    Buffer.ofLines(~font=Font.default(), lines_100k);

  let scm_100k_nochanges =
    Feature_SCM.initial
    |> Feature_SCM.setOriginalLines(buffer_100k_nochanges, lines_100k);
};

// TESTS

module Tests = {
  let diff_10k_nochanges = () => {
    let _ =
      EditorDiffMarkers.generate(
        ~scm=Data.scm_10k_nochanges,
        Data.buffer_10k_nochanges,
      );
    ();
  };

  let diff_10k_randomchanges = () => {
    let _ =
      EditorDiffMarkers.generate(
        ~scm=Data.scm_10k_randomchanges,
        Data.buffer_10k_randomchanges,
      );
    ();
  };

  let diff_10k_onelineoriginal = () => {
    let _ =
      EditorDiffMarkers.generate(
        ~scm=Data.scm_10k_onelineoriginal,
        Data.buffer_10k_onelineoriginal,
      );
    ();
  };

  let diff_10k_onelinemodified = () => {
    let _ =
      EditorDiffMarkers.generate(
        ~scm=Data.scm_10k_onelinemodified,
        Data.buffer_10k_onelinemodified,
      );
    ();
  };

  let diff_100k_nochanges = () => {
    let _ =
      EditorDiffMarkers.generate(
        ~scm=Data.scm_100k_nochanges,
        Data.buffer_100k_nochanges,
      );
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
