open EditorCoreTypes;

type t;

let create: (
    ~update: BufferUpdate.t,
    ~original: Buffer.t,
    ~updated: Buffer.t
) => t;

let apply: (
    ~shiftLines: (~afterLine: LineNumber.t, ~delta: int) => 'a,
    ~shiftCharacters: (
        ~line: LineNumber.t,
        ~afterByte: ByteIndex.t,
        ~deltaBytes: int,
    ) => 'a,
    t,
    'a
) => 'a;
