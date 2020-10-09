open Oni_Core;
open BenchFramework;

let longBufferLine = String.make(1000, 'A');

let setup = () => longBufferLine |> BufferLine.make(~measure=_ => 1.0);

bench(
  ~name="WordWrap: 1,000 character line (single character)",
  ~setup,
  ~f=
    longBufferLine => {
      let wrap = WordWrap.fixed(~pixels=100.);
      let _: list(WordWrap.lineWrap) = wrap(longBufferLine);
      ();
    },
  (),
);
