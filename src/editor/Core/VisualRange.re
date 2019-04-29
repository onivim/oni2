  [@deriving show({with_path: false})]
  type mode =
    | None
    | Visual /* "v" */
    | BlockwiseVisual /* "<C-v>" */
    | LinewiseVisual; /* "V" */

  [@deriving show({with_path: false})]
  type t = {
    range: Range.t,
    mode,
  };

  let _modeFromString = s => {
    switch (s) {
    | "V" => LinewiseVisual
    | "vb" => BlockwiseVisual
    | "v" => Visual
    | _ => None
    };
  };

  /*
   * The range might not always come through in the correct 'order' -
   * this method normalizes the range so that the (startLine, startColumn) is
   * before or equal to (endLine, endColumn)
   */
  let _normalizeRange = (startLine, startColumn, endLine, endColumn) =>
    if (startLine > endLine) {
      (endLine, endColumn, startLine, startColumn);
    } else if (startLine == endLine && startColumn > endColumn) {
      (endLine, endColumn, startLine, startColumn);
    } else {
      (startLine, startColumn, endLine, endColumn);
    };

  let create =
      (~startLine=1, ~startColumn=1, ~endLine=1, ~endColumn=1, ~mode="", ()) => {
    let (startLine, startColumn, endLine, endColumn) =
      _normalizeRange(startLine, startColumn, endLine, endColumn);

    let range =
      Range.create(
        ~startLine=OneBasedIndex(startLine),
        ~startCharacter=OneBasedIndex(startColumn),
        ~endLine=OneBasedIndex(endLine),
        ~endCharacter=OneBasedIndex(endColumn),
        (),
      );

    let mode = _modeFromString(mode);

    {range, mode};
  };
