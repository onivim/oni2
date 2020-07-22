open Oni_Core;
open BenchFramework;
open Feature_Editor;

let giantString =
  String.make(1000, 'a')
  |> BufferLine.make(~indentation=IndentationSettings.default);

let options = Reperf.Options.create(~iterations=1000, ());

let setup = () => ();

let indentationSettings = IndentationSettings.default;
let simpleColorizer = _ =>
  BufferLineColorizer.{
    color: Revery.Colors.black,
    backgroundColor: Revery.Colors.white,
    bold: false,
    italic: false,
  };

let tokenizeLine = () => {
  let _ =
    BufferViewTokenizer.tokenize(
      ~endIndex=1000,
      giantString,
      simpleColorizer,
    );
  ();
};

bench(
  ~name="BufferViewTokenizer: Tokenize line, no tokens",
  ~options,
  ~setup,
  ~f=tokenizeLine,
  (),
);
