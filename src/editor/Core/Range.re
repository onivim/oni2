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

let ofPositions = createFromPositions;

let create = (~startLine, ~startCharacter, ~endLine, ~endCharacter, ()) =>
  createFromPositions(
    ~startPosition=Position.create(startLine, startCharacter),
    ~endPosition=Position.create(endLine, endCharacter),
    (),
  );

let zero =
  create(
    ~startLine=ZeroBasedIndex(0),
    ~startCharacter=ZeroBasedIndex(0),
    ~endLine=ZeroBasedIndex(0),
    ~endCharacter=ZeroBasedIndex(0),
    (),
  );

let toZeroBasedPair = (v: Position.t) => {
  (Index.toZeroBasedInt(v.line), Index.toZeroBasedInt(v.character));
};

let explode = (measure, v) => {
  let (startLine, startCharacter) = v.startPosition |> toZeroBasedPair;
  let (endLine, endCharacter) = v.endPosition |> toZeroBasedPair;

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

let toHash = (ranges: list(t)) => {
  let rangeHash: Hashtbl.t(int, list(t)) = Hashtbl.create(100);
  List.iter(
    r => {
      let line = Index.toZeroBasedInt(r.startPosition.line);

      let newVal =
        switch (Hashtbl.find_opt(rangeHash, line)) {
        | Some(v) => [r, ...v]
        | None => [r]
        };

      Hashtbl.add(rangeHash, line, newVal);
    },
    ranges,
  );

  rangeHash;
};
