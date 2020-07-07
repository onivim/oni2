[@deriving show]
type t = {
  offset: int,
  searchText: string,
  maybeRemainingCount: option(int),
  items: list(Catalog.Summary.t),
};

let create = (~searchText) => {
  offset: 0,
  searchText,
  maybeRemainingCount: None,
  items: [],
};

let isComplete = ({maybeRemainingCount, _}) => {
  switch (maybeRemainingCount) {
  | None => false
  | Some(remainingCount) => remainingCount > 0
  };
};

let results = ({items, _}) => items;

let percentComplete = ({items, maybeRemainingCount, _}) => {
  switch (maybeRemainingCount) {
  | None => 0.
  | Some(c) => float(List.length(items)) /. float(List.length(items) + c)
  };
};
