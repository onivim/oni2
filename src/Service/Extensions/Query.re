type t = {
    searchText: string,
    maybeRemainingCount: option(int),
    items: list(Catalog.Summary.t),
};

let create = (~searchText) => {
    searchText: searchText,
    maybeRemainingCount: None,
    items: [],
};

let isComplete = ({items, maybeRemainingCount, _}) => {
    switch(maybeRemainingCount) {
    | None => false;
    | Some(remainingCount) => List.length(items) >= remainingCount
    }
};
 
let results = ({items, _}) => items;

let percentComplete = ({items, maybeRemainingCount, _}) => {
    switch (maybeRemainingCount) {
    | None => 0.
    | Some(c) => float(List.length(items)) /. float(c);
    }
};
