open EditorCoreTypes;

type element = {
  uniqueId: string,
  line: int, // Zero-based line index
  height: float,
};

type t = list(element);

let initial = [];

let compare = (a, b) => {
  a.line - b.line;
}

let remove = (~uniqueId: string, elements) => {
  elements
  |> List.filter(({uniqueId as origId, _}) => origId != uniqueId)
}

let add = (~uniqueId: string, ~line: LineNumber.t, ~height: float, elements) => {
  elements
  |> remove(~uniqueId)
  |> (filtered) => [{uniqueId, line: LineNumber.toZeroBased(line), height}, ...filtered]
  |> List.sort(compare)
};

let getReservedSpace = (line: int, elements: t) => {
  let lineNumber = LineNumber.toZeroBased(line);
  let rec loop = (acc, remainingElements) => {
    switch (remainingElements) {
    | [] => (acc, [])
    | [hd, ...tail] when hd.line <= lineNumber =>
      loop(acc +. hd.height, tail)
    | elementsPastLine => (acc, elementsPastLine)
    };
  };

  loop(0., elements)
};
