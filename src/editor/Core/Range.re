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

let ofInt0 = (~startLine, ~startCharacter, ~endLine, ~endCharacter, ()) =>
  create(
    ~startLine=ZeroBasedIndex(startLine),
    ~startCharacter=ZeroBasedIndex(startCharacter),
    ~endLine=ZeroBasedIndex(endLine),
    ~endCharacter=ZeroBasedIndex(endCharacter),
    (),
  );

let contains = (v: t, position: Position.t) => {
  let l0 = Index.toInt0(v.startPosition.line);
  let c0 = Index.toInt0(v.startPosition.character);
  let l1 = Index.toInt0(v.endPosition.line);
  let c1 = Index.toInt0(v.endPosition.character);

  let pl = Index.toInt0(position.line);
  let pc = Index.toInt0(position.character);

  (pl == l0 && pc >= c0 || pl > l0) && (pl == l1 && pc <= c1 || pl < l1);
};

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
