module Match: {
  type t = {
    file: string,
    text: string,
    lineNumber: int,
    charStart: int,
    charEnd: int,
  };
};

type t = {
  search:
    (
      ~followSymlinks: bool,
      ~useIgnoreFiles: bool,
      ~filesExclude: list(string),
      ~directory: string,
      ~onUpdate: list(string) => unit,
      ~onComplete: unit => unit,
      ~onError: string => unit
    ) =>
    dispose,
  findInFiles:
    (
      ~followSymlinks: bool,
      ~useIgnoreFiles: bool,
      ~searchExclude: list(string),
      ~searchInclude: list(string),
      ~directory: string,
      ~query: string,
      ~onUpdate: list(Match.t) => unit,
      ~onComplete: unit => unit,
      ~onError: string => unit,
      ~enableRegex: bool=?,
      ~caseSensitive: bool=?,
      unit
    ) =>
    dispose,
}

and dispose = unit => unit;

let make: (~executablePath: string) => t;
