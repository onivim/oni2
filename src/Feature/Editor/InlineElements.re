open EditorCoreTypes;

[@deriving show]
type element = {
  uniqueId: string,
  line: LineNumber.t,
  height: float,
};

[@deriving show]
type t = list(element);

let initial = [];

let compare = (a, b) => {
  LineNumber.toZeroBased(a.line) - LineNumber.toZeroBased(b.line);
};

let remove = (~uniqueId: string, elements) => {
  let idToRemove = uniqueId;
  elements |> List.filter(({uniqueId, _}) => uniqueId != idToRemove);
};

let add = (~uniqueId: string, ~line: LineNumber.t, ~height: float, elements) => {
  elements
  |> remove(~uniqueId)
  |> (
    filtered =>
      [{uniqueId, line, height}, ...filtered] |> List.sort(compare)
  );
};

let getReservedSpace = (line: LineNumber.t, elements: t) => {
  let lineNumber = LineNumber.toZeroBased(line);
  let rec loop = (acc, remainingElements) => {
    switch (remainingElements) {
    | [] => acc
    | [hd, ...tail] when LineNumber.toZeroBased(hd.line) <= lineNumber =>
      loop(acc +. hd.height, tail)
    | _elementsPastLine => acc
    };
  };

  loop(0., elements);
};
