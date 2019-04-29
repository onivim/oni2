/*
 * BufferWrap.re
 *
 * Data-structure for maintaining line-wrap information
 * for a particular buffer / view
 */

open Oni_Core;

type t = {
	bufferLineToVirtualLine: IntMap.t(LineWrap.t),
	virtualLineToBufferLine: IntMap.t(int),
	virtualLines: int,
};

let empty = {
	bufferLineToVirtualLine: IntMap.empty,
	virtualLineToBufferLine: IntMap.empty,
	virtualLines: 0,
};

let create = (buffer: Buffer.t, wrapColumn: int) => {
	let lineCount = Buffer.getNumberOfLines(buffer);

	let bufferLineMap: ref(IntMap.t(LineWrap.t)) = ref(IntMap.empty);

	let idx = ref(0);
	let lineCount = ref(0);
	while (idx < lineCount) {

		let i = idx^;
		let line = Buffer.getLine(buffer, i);

		let lineWrap = LineWrap.create(line, wrapColumn);
		bufferLineMap := IntMap.add(i, lineWrap, bufferLineMap^);

		lineCount := lineCount^ + LineWrap.count(lineWrap);

		incr(idx);
	};

	{
		bufferLineToVirtualLine: bufferLineMap^,
		virtualLineToBufferLine: IntMap.empty,
		virtualLines: lineCount^,
	};
};
