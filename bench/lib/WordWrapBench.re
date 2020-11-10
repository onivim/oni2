open Oni_Core;
open BenchFramework;

let longBufferLine = String.make(1000 * 1024, 'A');

let setup = () => longBufferLine |> BufferLine.make(~measure=_ => 1.0);

let options = Reperf.Options.create(~iterations=1, ());

bench(
  ~name="WordWrap: 1,000 character line (single character)",
  ~setup,
  ~options,
  ~f=
    longBufferLine => {
      let wrap = WordWrap.fixed(~pixels=100.);
      let _: array(WordWrap.lineWrap) = wrap(longBufferLine);
      ();
    },
  (),
);
