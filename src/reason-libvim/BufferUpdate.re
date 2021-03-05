type t = {
  id: int,
  startLine: int,
  endLine: int,
  lines: array(string),
  version: int,
};

let show = (v: t) => {
  Printf.sprintf(
    "id: %d startLine: %d endLine: %d version: %d lines:\n",
    v.id,
    v.startLine,
    v.endLine,
    v.version,
  )
  ++ "---- "
  ++ string_of_int(Array.length(v.lines))
  ++ "\n"
  ++ Array.fold_left((s, prev) => prev ++ "\n" ++ s, "", v.lines)
  ++ "\n----";
};

let getAllLines = (buffer: Native.buffer) => {
  let startLine = 1;
  let endLine = Native.vimBufferGetLineCount(buffer) + 1;

  let idx = ref(startLine);
  let max = endLine;
  let count = endLine - startLine;
  let lines = Array.make(count, "");

  while (idx^ < max) {
    let i = idx^;
    let line = Native.vimBufferGetLine(buffer, i);
    lines[i - startLine] = line;
    incr(idx);
  };

  lines;
};

let createInitial = (buffer: Native.buffer) => {
  let id = Native.vimBufferGetId(buffer);
  let version = Native.vimBufferGetChangedTick(buffer) - 1;
  let lines = getAllLines(buffer);

  {id, startLine: 1, endLine: (-1), lines, version};
};

let createIncremental =
    (~buffer: Native.buffer, ~startLine, ~endLine, ~extra: int) => {
  let id = Native.vimBufferGetId(buffer);
  let version = Native.vimBufferGetChangedTick(buffer);
  let idx: ref(int) = ref(startLine);

  let max = endLine + extra;
  let count = max - startLine;

  let lines = Array.make(count, "");

  while (idx^ < max) {
    let i = idx^;
    let line = Native.vimBufferGetLine(buffer, i);
    lines[i - startLine] = line;
    incr(idx);
  };

  {id, startLine, endLine, lines, version};
};

let createFull = (buffer: Native.buffer) => {
  let id = Native.vimBufferGetId(buffer);
  let version = Native.vimBufferGetChangedTick(buffer);

  let lines = getAllLines(buffer);

  {id, startLine: 1, endLine: (-1), lines, version};
};
