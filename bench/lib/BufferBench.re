open Oni_Core;
open BenchFramework;

open Revery.UI;

let rootNode = (new node)();

let setup = () => ();

let emptyBuffer = Buffer.ofLines([||]);
let emptyBufferId = Buffer.getMetadata(emptyBuffer).id;

let hundredThousandLineBuffer =
  Buffer.ofLines(Array.make(100000, "This buffer is pretty big"));
let hundredThousandLineBufferId =
  Buffer.getMetadata(hundredThousandLineBuffer).id;

let smallBuffer =
  Buffer.ofLines(Array.make(100, "This buffer is a bit smaller"));
let smallBufferId = Buffer.getMetadata(smallBuffer).id;

let hundredThousandLines =
  Array.make(100000, "Another big buffer update") |> Array.to_list;

let addLinesToEmptyBuffer = () => {
  let _ =
    Types.BufferUpdate.create(
      ~id=emptyBufferId,
      ~startLine=0,
      ~endLine=-1,
      ~lines=hundredThousandLines,
      ~version=1,
      (),
    )
    |> Buffer.update(emptyBuffer);
  ();
};

let clearLargeBuffer = () => {
  let _ =
    Types.BufferUpdate.create(
      ~id=hundredThousandLineBufferId,
      ~startLine=0,
      ~endLine=-1,
      ~lines=[],
      ~version=1,
      (),
    )
    |> Buffer.update(hundredThousandLineBuffer);
  ();
};

let insertInMiddleOfSmallBuffer = () => {
  let _ =
    Types.BufferUpdate.create(
      ~id=smallBufferId,
      ~startLine=50,
      ~endLine=51,
      ~lines=["this is a new line"],
      ~version=1,
      (),
    )
    |> Buffer.update(smallBuffer);
  ();
};

let insertInMiddleOfLargeBuffer = () => {
  let _ =
    Types.BufferUpdate.create(
      ~id=hundredThousandLineBufferId,
      ~startLine=5000,
      ~endLine=50001,
      ~lines=["this is a new line"],
      ~version=1,
      (),
    )
    |> Buffer.update(hundredThousandLineBuffer);
  ();
};

let options = Reperf.Options.create(~iterations=1000, ());

bench(
  ~name="Buffer: Add lines to empty buffer",
  ~options,
  ~setup,
  ~f=addLinesToEmptyBuffer,
  (),
);
bench(
  ~name="Buffer: Insert line in middle of small buffer",
  ~options,
  ~setup,
  ~f=insertInMiddleOfSmallBuffer,
  (),
);
bench(
  ~name="Buffer: Insert line in middle of large buffer",
  ~options,
  ~setup,
  ~f=insertInMiddleOfLargeBuffer,
  (),
);
bench(
  ~name="Buffer: Clear large buffer",
  ~options,
  ~setup,
  ~f=clearLargeBuffer,
  (),
);
