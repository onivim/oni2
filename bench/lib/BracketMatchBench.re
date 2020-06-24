open Feature_Editor;

open BenchFramework;

let createLargeEditorBuffer = () =>
  String.make(1000, 'a')
  |> Array.make(10000)
  |> Oni_Core.Buffer.ofLines
  |> Feature_Editor.EditorBuffer.ofBuffer; 


let setup = () => createLargeEditorBuffer();

let options = Reperf.Options.create(~iterations=1000, ());

let bracketFind = (buffer) => {
  let _: option(BracketMatch.pair) = BracketMatch.find(
    ~buffer,
    ~line=5000,
    ~index=0,
    ~start=Uchar.of_char('{'),
    ~stop=Uchar.of_char('}')
  );
}

bench(~name="BracketMatch: findPair", ~options, ~setup, ~f=bracketFind, ());
