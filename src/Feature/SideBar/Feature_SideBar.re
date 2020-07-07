type pane =
  | FileExplorer
  | SCM
  | Extensions;

[@deriving show]
type msg =
  | ResizeInProgress(int)
  | ResizeCommitted;

module Constants = {
  let defaultWidth = 225;
  let minWidth = 50;
  let maxWidth = 800;
};

type model = {
  // Track the last value of 'workbench.sideBar.visible'
  // If it changes, we should update
  openByDefault: bool,
  isOpen: bool,
  selected: pane,
  width: int,
  resizeDelta: int,
};

let selected = ({selected, _}) => selected;
let isOpen = ({isOpen, _}) => isOpen;

let initial = {
  openByDefault: false,
  isOpen: false,
  selected: FileExplorer,
  width: Constants.defaultWidth,
  resizeDelta: 0,
};

let width = ({width, resizeDelta, _}) => {
  let candidate = width + resizeDelta;
  if (candidate < Constants.minWidth) {
    0;
  } else if (candidate > Constants.maxWidth) {
    Constants.maxWidth;
  } else {
    candidate;
  };
};

let update = (msg, model) =>
  switch (msg) {
  | ResizeInProgress(delta) => {...model, resizeDelta: delta}
  | ResizeCommitted =>
    let newWidth = width(model);
    if (newWidth == 0) {
      {...model, isOpen: false, resizeDelta: 0};
    } else {
      {...model, width: newWidth, resizeDelta: 0};
    };
  };

let isVisible = (pane, model) => {
  model.isOpen && model.selected == pane;
};

let setDefaultVisibility = (pane, defaultVisibility) =>
  if (pane.openByDefault == defaultVisibility) {
    pane;
  } else {
    {...pane, openByDefault: defaultVisibility, isOpen: defaultVisibility};
  };

let toggle = (pane, state) =>
  if (pane == state.selected) {
    {...state, isOpen: !state.isOpen};
  } else {
    {...state, isOpen: true, selected: pane};
  };
