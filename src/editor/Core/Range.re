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

  let zero =
    create(
      ~startLine=ZeroBasedIndex(0),
      ~startCharacter=ZeroBasedIndex(0),
      ~endLine=ZeroBasedIndex(0),
      ~endCharacter=ZeroBasedIndex(0),
      (),
    );
