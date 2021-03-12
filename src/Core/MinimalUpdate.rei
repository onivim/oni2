open EditorCoreTypes;

type update =
  | Added({
      beforeLine: LineNumber.t,
      lines: array(string),
    })
  | Deleted({
      startLine: LineNumber.t,
      stopLine: LineNumber.t,
    })
  | Modified({
      line: LineNumber.t,
      original: string,
      updated: string,
    });

type t;

let toDebugString: t => string;

let map: (update => 'a, t) => list('a);

let fromBuffers: (~original: Buffer.t, ~updated: Buffer.t) => t;

let fromBufferUpdate: (~buffer: Buffer.t, ~update: BufferUpdate.t) => t;
