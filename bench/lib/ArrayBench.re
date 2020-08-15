open BenchFramework;

let emptyArray = [||];

let testArray = [||];

let setup = () => ();

let referenceEqualityBench = () => {
  Sys.opaque_identity(testArray === emptyArray) |> ignore;
};

let structuralEqualityBench = () => {
  Sys.opaque_identity(testArray == [||]) |> ignore;
};

bench(
  ~name="Array: structural equality check",
  ~setup,
  ~f=structuralEqualityBench,
  (),
);

bench(
  ~name="Array: reference equality check",
  ~setup,
  ~f=referenceEqualityBench,
  (),
);
