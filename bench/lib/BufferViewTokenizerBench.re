open Oni_Core;
open Oni_Extensions;    
open Oni_Model;
open BenchFramework;

let giantString = String.make(1000, 'a');

let theme = Theme.create();
let noTokens: list(ColorizedToken.t) = [];
let colorMap = ColorMap.create();

let options = Reperf.Options.create(~iterations=1000, ());

let setup = () => ();

let tokenizeLine = () => {
    let _ = BufferViewTokenizer.tokenize(giantString, theme, noTokens, colorMap);
};

bench(
  ~name="BufferViewTokenizer: Tokenize line, no tokens",
  ~options,
  ~setup,
  ~f=tokenizeLine,
  (),
);
