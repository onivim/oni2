type t = {
  anchor: int,
  focus: int
};

type selectionVariant =
  | Left(string, int)
  | Right(string, int)
  | Start
  | End(string)
  | Position(string, int);

let initial:t = {
  anchor: 0,
  focus: 0
};


let withinLength = (text: string, position: int):int => {
  let withingStart = max(position, 0);
  let withingBoth = min(withingStart, String.length(text));

  withingBoth;
}

let create = (text:string, ~anchor: int, ~focus: int):t => {
  let safeAnchor = withinLength(text, anchor);
  let safeFocus = withinLength(text, focus);

  { anchor: safeAnchor, focus: safeFocus };
}

let isCollapsed = (selection: t): bool => {
  selection.anchor == selection.focus;
};

let collapse = (selection: t, direction: selectionVariant): t => {
  switch (direction) {
    | Left(text, offset) => {
      let start = min(selection.anchor, selection.focus);
      let collapsed = max(start - abs(offset), 0);

      { anchor: collapsed, focus: collapsed };
    };
    | Right(text, offset) => {
      let ending = max(selection.anchor, selection.focus);
      let collapsed = min(ending + abs(offset), String.length(text));

      { anchor: collapsed, focus: collapsed };
    };
    | Start => initial;
    | End(text) => {
      let collapsed = String.length(text);

      { anchor: collapsed, focus: collapsed };
    };
    | Position(text, offset) => {
      let safeOffset = withinLength(text, offset);

      { anchor: safeOffset, focus: safeOffset };
    };
  };
};


let extend = (selection: t, direction: selectionVariant ) => {
  switch (direction) {
    | Left(text, offset) => {
      let extended = max(selection.focus - abs(offset), 0);

      { anchor: selection.anchor, focus: extended };
    }
    | Right(text, offset) => {
      let extended = min(selection.focus + abs(offset), String.length(text));

      { anchor: selection.anchor, focus: extended };
    }
    | Start => { anchor: selection.anchor, focus: 0 };
    | End(text) => {
      let ending = String.length(text);

      { anchor: selection.anchor, focus: ending };
    }
    | Position(text, offset) => {
      let safeOffset = withinLength(text, offset);

      { anchor: selection.anchor, focus: safeOffset };
    };
  };
};
