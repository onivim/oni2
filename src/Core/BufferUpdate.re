module Index = Types.Index;

type t = {
  id: int,
  startCharacter: Index.t,
  startLine: Index.t,
  oldEndCharacter: Index.t,
  oldEndLine: Index.t,
  newEndCharacter: Index.t,
  newEndLine: Index.t,
  lines: array(string),
  version: int,
};

let create =
    (
      ~id,
      ~startCharacter,
      ~startLine,
      ~oldEndCharacter,
      ~oldEndLine,
      ~newEndCharacter,
      ~newEndLine,
      ~lines,
      ~version,
      (),
    ) => {
  let ret: t = {
    id,
    startCharacter,
    startLine,
    oldEndCharacter,
    oldEndLine,
    newEndCharacter,
    newEndLine,
    lines,
    version,
  };
  ret;
};

let toString = bufferUpdate => {
  let lines = bufferUpdate.lines |> Array.to_list |> String.concat("|\n");
  Printf.sprintf(
    "[BufferUpdate] - id: %d\n -version: %d\n -startCharacter: %d\n -startLine: %d\n -oldEndCharacter: %d\n -oldEndLine: %d\n -newEndCharacter: %d\n -newEndLine: %d\n lines: ===\n%s=== ",
    bufferUpdate.id,
    bufferUpdate.version,
    bufferUpdate.startCharacter |> Index.toInt0,
    bufferUpdate.startLine |> Index.toInt0,
    bufferUpdate.oldEndCharacter |> Index.toInt0,
    bufferUpdate.oldEndLine |> Index.toInt0,
    bufferUpdate.newEndCharacter |> Index.toInt0,
    bufferUpdate.newEndLine |> Index.toInt0,
    lines,
  );
};
