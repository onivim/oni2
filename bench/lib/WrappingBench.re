open Oni_Core;
open EditorCoreTypes;
open BenchFramework;

open Feature_Editor;

let longBufferLine = String.make(20 * 1024 * 1024, 'A');

let buffer =
  Buffer.ofLines(~font=Font.default(), [|longBufferLine|])
  |> EditorBuffer.ofBuffer;

let setup = () => ();

let options = Reperf.Options.create(~iterations=1, ());

bench(
  ~name="Wrapping: Initial creation from long buffer line",
  ~options,
  ~setup,
  ~f=
    () => {
      let _initial: Wrapping.t =
        Wrapping.make(~wrap=WordWrap.fixed(~pixels=100.), ~buffer);
      ();
    },
  (),
);

let wrapping = Wrapping.make(~wrap=WordWrap.fixed(~pixels=100.), ~buffer);

bench(
  ~name="Wrapping: bufferBytePositionToViewLine",
  ~options,
  ~setup,
  ~f=
    () => {
      let _viewLine: int =
        Wrapping.bufferBytePositionToViewLine(
          ~bytePosition=
            BytePosition.{
              byte: ByteIndex.ofInt(20 * 1024 * 1024 - 1),
              line: LineNumber.ofZeroBased(0),
            },
          wrapping,
        );
      ();
    },
  (),
);
