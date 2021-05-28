open EditorCoreTypes;

type t;

let create: MinimalUpdate.t => t;

let apply:
  (
    ~clearLine: (~line: LineNumber.t, 'a) => 'a,
    ~shiftLines: (~afterLine: LineNumber.t, ~delta: int, 'a) => 'a,
    ~shiftCharacters: (
                        ~line: LineNumber.t,
                        ~afterByte: ByteIndex.t,
                        ~deltaBytes: int,
                        ~afterCharacter: CharacterIndex.t,
                        ~deltaCharacters: int,
                        'a
                      ) =>
                      'a,
    t,
    'a
  ) =>
  'a;

let toDebugString: t => string;
