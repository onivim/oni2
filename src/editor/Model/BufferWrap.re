/*
 * BufferWrap.re
 *
 * Data-structure for maintaining line-wrap information
 * for a particular buffer / view
 */

open Oni_Core;
open Oni_Core.Types;

type t = {
  bufferLineToVirtualLine: IntMap.t(LineWrap.t),
  virtualLineToBufferLine: IntMap.t(int),
  virtualLines: int,
};

let create = (buffer, wrapColumn) => {
  let lineCount = Buffer.getNumberOfLines(buffer);

  let bufferLineMap: ref(IntMap.t(LineWrap.t)) = ref(IntMap.empty);

  let idx = ref(0);
  let virtualLineCount = ref(0);
  while (idx^ < lineCount) {
    let i = idx^;
    let line = Buffer.getLine(buffer, i);

    let lineWrap = LineWrap.create(line, wrapColumn);
    bufferLineMap := IntMap.add(i, lineWrap, bufferLineMap^);

    virtualLineCount := virtualLineCount^ + LineWrap.count(lineWrap);

    incr(idx);
  };

  {
    bufferLineToVirtualLine: bufferLineMap^,
    virtualLineToBufferLine: IntMap.empty,
    virtualLines: virtualLineCount^,
  };
};

let getVirtualLine = (_idx, _buffer, _v) => {
  "";
};

let getVirtualLineCount = v => v.virtualLines;

let bufferPositionToVirtual = (_position, _v) => {
  Position.zero;
};

let bufferRangeToVirtual = (range, _v) => {
  [range];
};

let update = (_update, v) => v;
