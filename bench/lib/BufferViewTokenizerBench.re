open Oni_Core;
open Oni_Model;
open BenchFramework;
open Revery;

let giantString =
  String.make(1000, 'a')
  |> BufferLine.make(~indentation=IndentationSettings.default);

let options = Reperf.Options.create(~iterations=1000, ());

let setup = () => ();

let indentationSettings = IndentationSettings.default;
let simpleColorizer = _ => (Colors.black, Colors.white);

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
