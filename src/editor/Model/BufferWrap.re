/*
 * BufferWrap.re
 *
 * Data-structure for maintaining line-wrap information
 * for a particular buffer / view
 */

open Oni_Core;
open Oni_Core.Types;

type virtualLineInfo = {
    virtualStartLine: int,
	lineWrap: LineWrap.t
};

type t = {
  bufferLineToVirtualInfo: IntHash.t(virtualLineInfo),
  virtualLineToBufferLine: IntHash.t(int),
  virtualLines: int,
};

let create = (buffer, wrapColumn) => {
  let lineCount = Buffer.getNumberOfLines(buffer);

  /* let bufferLineMap: ref(IntMap.t(LineWrap.t)) = ref(IntMap.empty); */

  let virtualLineHash: IntHash.t(int) = IntHash.create(lineCount);
  let bufferLineHash: IntHash.t(virtualLineInfo) = IntHash.create(lineCount);

  let idx = ref(0);
  let virtualLineCount = ref(0);
  while (idx^ < lineCount) {
    let i = idx^;
    let line = Buffer.getLine(buffer, i);

    let lineWrap = LineWrap.create(line, wrapColumn);
	let vlc = virtualLineCount^;

    let vInfo = {
				   virtualStartLine: vlc,
				   lineWrap,
				};

    IntHash.add(bufferLineHash, i, vInfo);

    
    virtualLineCount := vlc + LineWrap.count(lineWrap);

	let curr = ref(vlc);

	while (curr^ < virtualLineCount^) {
		IntHash.add(virtualLineHash, curr^, i);
		incr(curr);
	}

    incr(idx);
  };

  {
    bufferLineToVirtualInfo: bufferLineHash,
    virtualLineToBufferLine: virtualLineHash,
    virtualLines: virtualLineCount^,
  };
};

let getVirtualLine = (idx, buffer, v) => {
	switch (IntHash.find_opt(v.virtualLineToBufferLine, idx)) {
	| None => ""
	| Some(bufferLineIdx) => {
		switch (IntHash.find_opt(v.bufferLineToVirtualInfo, bufferLineIdx)) {
		| None => ""
		| Some(vInfo) => {
			let line = Buffer.getLine(buffer, bufferLineIdx);
			prerr_endline ("WHOLE LINE: " ++ line);
			let virtualLineStart = vInfo.virtualStartLine;
			let arrayOffset = idx - virtualLineStart;

			let (idx, length) = LineWrap.getOffsets(arrayOffset, vInfo.lineWrap);
			prerr_endline ("IDX: " ++ string_of_int(idx));
			prerr_endline ("LENGTH: " ++ string_of_int(length));
			Zed_utf8.sub(line, idx, length);
		}
		}
	}
	}
};

let getVirtualLineCount = v => v.virtualLines;

let bufferPositionToVirtual = (_position, _v) => {
  Position.zero;
};

let bufferRangeToVirtual = (range, _v) => {
  [range];
};

let update = (_update, v) => v;
