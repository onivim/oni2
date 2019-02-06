module Position = {
  type t =
    | ZeroBasedPosition(int)
    | OneBasedPosition(int);

  let toZeroBasedIndex = (pos: t) =>
    switch (pos) {
    | ZeroBasedPosition(n) => n
    | OneBasedPosition(n) => n - 1
    };
};

module EditorFont = {
  type t = {
    fontFile: string,
    fontSize: int,
    measuredWidth: int,
    measuredHeight: int,
  };

  let create = (~fontFile, ~fontSize, ~measuredWidth, ~measuredHeight, ()) => {
    fontFile,
    fontSize,
    measuredWidth,
    measuredHeight,
  };
};
