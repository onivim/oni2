open Types;

[@deriving show({with_path: false})]
type t = {
  startPosition: Position.t,
  endPosition: Position.t,
};

let createFromPositions = (~startPosition, ~endPosition, ()) => {
  startPosition,
  endPosition,
};

let create = (~startLine, ~startCharacter, ~endLine, ~endCharacter, ()) =>
  createFromPositions(
    ~startPosition=Position.create(startLine, startCharacter),
    ~endPosition=Position.create(endLine, endCharacter),
    (),
  );

let ofInt0 = (~startLine, ~startCharacter, ~endLine, ~endCharacter, ()) => {
  create(
    ~startLine=ZeroBasedIndex(startLine),
    ~startCharacter=ZeroBasedIndex(startCharacter),
    ~endLine=ZeroBasedIndex(endLine),
    ~endCharacter=ZeroBasedIndex(endCharacter),
    (),
  );
};

let ofPositions = createFromPositions;

let zero =
  create(
    ~startLine=ZeroBasedIndex(0),
    ~startCharacter=ZeroBasedIndex(0),
    ~endLine=ZeroBasedIndex(0),
    ~endCharacter=ZeroBasedIndex(0),
    (),
  );

let explode = (measure, v) => {
  let (startLine, startCharacter) = v.startPosition |> Position.toInt0;
  let (endLine, endCharacter) = v.endPosition |> Position.toInt0;

  if (startLine == endLine) {
    [v];
  } else {
    let idx = ref(startLine);

    let ranges = ref([]);

    while (idx^ < endLine) {
      let i = idx^;

      let startCharacter = i == startLine ? startCharacter : 0;
      let endCharacter = max(0, measure(i) - 1);

      ranges :=
        [
          create(
            ~startLine=ZeroBasedIndex(i),
            ~startCharacter=ZeroBasedIndex(startCharacter),
            ~endCharacter=ZeroBasedIndex(endCharacter),
            ~endLine=ZeroBasedIndex(i),
            (),
          ),
          ...ranges^,
        ];

      incr(idx);
    };

    [
      create(
        ~startLine=ZeroBasedIndex(endLine),
        ~startCharacter=ZeroBasedIndex(0),
        ~endCharacter=ZeroBasedIndex(endCharacter),
        ~endLine=ZeroBasedIndex(endLine),
        (),
      ),
      ...ranges^,
    ]
    |> List.rev;
  };
};
