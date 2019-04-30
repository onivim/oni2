/*
 * BufferWrap.re
 *
 * Data-structure for maintaining line-wrap information
 * for a particular buffer / view
 */

exception BufferWrapOutOfRangePosition;
exception BufferOutOfSync;

open Oni_Core;
open Oni_Core.Types;

type virtualLineInfo = {
  virtualStartLine: int,
  lineWrap: LineWrap.t,
};

type t = {
  bufferId: int,
  bufferVersion: int,
  bufferLineToVirtualInfo: IntHash.t(virtualLineInfo),
  virtualLineToBufferLine: IntHash.t(int),
  virtualLines: int,
};

let create = (buffer, wrapMode) => {
  let lineCount = Buffer.getNumberOfLines(buffer);

  let {id, version, _}: BufferMetadata.t = Buffer.getMetadata(buffer);

  /* let bufferLineMap: ref(IntMap.t(LineWrap.t)) = ref(IntMap.empty); */

  let virtualLineHash: IntHash.t(int) = IntHash.create(lineCount);
  let bufferLineHash: IntHash.t(virtualLineInfo) = IntHash.create(lineCount);

  let idx = ref(0);
  let virtualLineCount = ref(0);
  while (idx^ < lineCount) {
    let i = idx^;
    let line = Buffer.getLine(buffer, i);

    let lineWrap = LineWrap.create(line, wrapMode);
    let vlc = virtualLineCount^;

    let vInfo = {virtualStartLine: vlc, lineWrap};

    IntHash.add(bufferLineHash, i, vInfo);

    virtualLineCount := vlc + LineWrap.count(lineWrap);

    let curr = ref(vlc);

    while (curr^ < virtualLineCount^) {
      IntHash.add(virtualLineHash, curr^, i);
      incr(curr);
    };

    incr(idx);
  };

  {
	bufferId: id,
	bufferVersion: version,
    bufferLineToVirtualInfo: bufferLineHash,
    virtualLineToBufferLine: virtualLineHash,
    virtualLines: virtualLineCount^,
  };
};

let assertBufferMatches = (buffer, v) => {
	let {id, version, _}: BufferMetadata.t = Buffer.getMetadata(buffer);
	let {bufferId, bufferVersion, _} = v;

	switch (id == bufferId && version == bufferVersion) {
	| false => raise(BufferOutOfSync)
	| _ => ();
	}
};

let getVirtualLine = (idx, buffer, v) => {
  assertBufferMatches(buffer, v);

  switch (IntHash.find_opt(v.virtualLineToBufferLine, idx)) {
  | None => ""
  | Some(bufferLineIdx) =>
    switch (IntHash.find_opt(v.bufferLineToVirtualInfo, bufferLineIdx)) {
    | None => ""
    | Some(vInfo) =>
      let line = Buffer.getLine(buffer, bufferLineIdx);
      let virtualLineStart = vInfo.virtualStartLine;
      let arrayOffset = idx - virtualLineStart;

      let (idx, length) =
        LineWrap.getOffsets(Index.ofInt0(arrayOffset), vInfo.lineWrap);
      Zed_utf8.sub(line, idx, length);
    }
  };
};

let getVirtualLineLength = (idx, v) => {
  switch (IntHash.find_opt(v.virtualLineToBufferLine, idx)) {
  | None => 0
  | Some(bufferLineIdx) =>
    switch (IntHash.find_opt(v.bufferLineToVirtualInfo, bufferLineIdx)) {
    | None => 0
    | Some(vInfo) =>
      let virtualLineStart = vInfo.virtualStartLine;
      let arrayOffset = idx - virtualLineStart;
      let (_, length) =
        LineWrap.getOffsets(Index.ofInt0(arrayOffset), vInfo.lineWrap);
      length;
    }
  };
};

let getVirtualLineCount = v => v.virtualLines;

let bufferPositionToVirtual = (position, v) => {
  let (line0, character0) = Position.toInt0(position);

  switch (IntHash.find_opt(v.bufferLineToVirtualInfo, line0)) {
  | None => raise(BufferWrapOutOfRangePosition)
  | Some(v) =>
    let {virtualStartLine, lineWrap} = v;
    let (lineOffset, character) =
      LineWrap.toVirtualPosition(Index.ofInt0(character0), lineWrap)
      |> Position.toInt0;
    Position.ofInt0(virtualStartLine + lineOffset, character);
  };
};

let addOneCharacter = range => {
  open Range;

  let {startPosition, endPosition} = range;

  let (sl, sc) = Position.toInt0(startPosition);
  let (el, ec) = Position.toInt0(endPosition);

  Range.ofInt0(
    ~startLine=sl,
    ~startCharacter=sc,
    ~endLine=el,
    ~endCharacter=ec + 1,
    (),
  );
};

let bufferRangeToVirtual = (range: Range.t, v) => {
  open Range;
  let {startPosition, endPosition} = range;

  let vStartPos = bufferPositionToVirtual(startPosition, v);
  let vEndPos = bufferPositionToVirtual(endPosition, v);

  if (Index.equal(vStartPos.line, vEndPos.line)) {
    [Range.ofPositions(~startPosition=vStartPos, ~endPosition=vEndPos, ())];
  } else {
    let virtualRange =
      Range.ofPositions(~startPosition=vStartPos, ~endPosition=vEndPos, ())
      |> addOneCharacter;

    let ranges =
      Range.explode(l => getVirtualLineLength(l, v) + 1, virtualRange);
    ranges;
  };
};

let update = (_update, v) => v;
