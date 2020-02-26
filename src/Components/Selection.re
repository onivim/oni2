[@deriving show({with_path: false})]
type t = {
  anchor: int,
  focus: int
};

type directionVariant =
  | Left(int)
  | Right(int);

let initial:t = {
  anchor: 0,
  focus: 0
};


let withinLength = (text: string, position: int):int => {
  let withingStart = max(position, 0);
  let withingBoth = min(withingStart, String.length(text));

  withingBoth;
}

let create = (~text:string, ~anchor: int, ~focus: int):t => {
  let safeAnchor = withinLength(text, anchor);
  let safeFocus = withinLength(text, focus);

  { anchor: safeAnchor, focus: safeFocus };
}

let anchor = (selection: t):  int => {
  selection.anchor;
}

let focus = (selection: t):  int => {
  selection.focus;
}

let range = (selection: t): int => {
  abs(selection.focus - selection.anchor);
}

let rangeStart = (selection: t): int => {
  min(selection.focus, selection.anchor);
}

let rangeEnd = (selection: t): int => {
  max(selection.focus, selection.anchor);
}

let isCollapsed = (selection: t): bool => {
  selection.anchor == selection.focus;
};

let collapse = (~text:string, offset:int):t => {
  create(~text, ~anchor=offset, ~focus=offset)
};

let collapseRelative = (~text:string, ~selection:t, direction:directionVariant):t => {
  switch (direction) {
    | Left(offset) => {
      collapse(~text, rangeStart(selection) - abs(offset));
    };
    | Right(offset) => {
      collapse(~text, rangeEnd(selection) + abs(offset))
    };
  }
}


let extend = (~text:string, ~selection:t, offset:int):t => {
  create(~text, ~anchor=anchor(selection), ~focus=offset);
};

let extendRelative = (~text:string, ~selection:t, direction:directionVariant):t => {
  switch (direction) {
    | Left(offset) => {
      extend(~text, ~selection, focus(selection) - abs(offset))
    };
    | Right(offset) => {
      extend(~text, ~selection, focus(selection) + abs(offset))
    };
  }
}
