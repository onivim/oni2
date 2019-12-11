open Oni_Core;
open BenchFramework;

open Revery.UI;

let rootNode = (new node)();

let setup = () => ();

let emptyBuffer = Buffer.ofLines([||]);
let emptyBufferId = Buffer.getId(emptyBuffer);

let hundredThousandLineBuffer =
  Buffer.ofLines(Array.make(100000, "This buffer is pretty big"));
let hundredThousandLineBufferId = Buffer.getId(hundredThousandLineBuffer);

let smallBuffer =
  Buffer.ofLines(Array.make(100, "This buffer is a bit smaller"));
let smallBufferId = Buffer.getId(smallBuffer);

let hundredThousandLines = Array.make(100000, "Another big buffer update");

let addLinesToEmptyBuffer = () => {
  let _ =
    BufferUpdate.create(
      ~id=emptyBufferId,
      ~startLine=Index.ZeroBasedIndex(0),
      ~endLine=Index.ZeroBasedIndex(-1),
      ~lines=hundredThousandLines,
      ~version=1,
      (),
    )
    |> Buffer.update(emptyBuffer);
  ();
};

let clearLargeBuffer = () => {
  let _ =
    BufferUpdate.create(
      ~id=hundredThousandLineBufferId,
      ~startLine=Index.ZeroBasedIndex(0),
      ~endLine=Index.ZeroBasedIndex(-1),
      ~lines=[||],
      ~version=1,
      (),
    )
    |> Buffer.update(hundredThousandLineBuffer);
  ();
};

let insertInMiddleOfSmallBuffer = () => {
  let _ =
    BufferUpdate.create(
      ~id=smallBufferId,
      ~startLine=Index.ZeroBasedIndex(50),
      ~endLine=Index.ZeroBasedIndex(51),
      ~lines=[|"this is a new line"|],
      ~version=1,
      (),
    )
    |> Buffer.update(smallBuffer);
  ();
};

let insertInMiddleOfLargeBuffer = () => {
  let _ =
    BufferUpdate.create(
      ~id=hundredThousandLineBufferId,
      ~startLine=Index.ZeroBasedIndex(5000),
      ~endLine=Index.ZeroBasedIndex(50001),
      ~lines=[|"this is a new line"|],
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
