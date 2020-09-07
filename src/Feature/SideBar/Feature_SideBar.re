type pane =
  | FileExplorer
  | SCM
  | Extensions
  | Search;

type location =
  | Left
  | Right;

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
  shouldSnapShut: bool,
  location,
};

let selected = ({selected, _}) => selected;
let isOpen = ({isOpen, _}) => isOpen;
let location = ({location, _}) => location;

let initial = {
  openByDefault: false,
  isOpen: false,
  selected: FileExplorer,
  width: Constants.defaultWidth,
  resizeDelta: 0,
  shouldSnapShut: true,
  location: Left,
};

let width = ({width, resizeDelta, isOpen, location, shouldSnapShut, _}) =>
  if (!isOpen) {
    0;
  } else {
    let candidate =
      switch (location) {
      | Left => width + resizeDelta
      | Right => width - resizeDelta
      };

    if (candidate < Constants.minWidth && shouldSnapShut) {
      0;
    } else if (candidate > Constants.maxWidth) {
      Constants.maxWidth;
    } else {
      max(0, candidate);
    };
  };

let update = (msg, model) =>
  switch (msg) {
  | ResizeInProgress(delta) =>
    if (model.isOpen) {
      {...model, resizeDelta: delta};
    } else {
      {
        ...model,
        width: 0,
        resizeDelta: delta,
        isOpen: true,
        shouldSnapShut: false,
      };
    }
  | ResizeCommitted =>
    let newWidth = width(model);
    if (newWidth == 0) {
      {...model, isOpen: false, shouldSnapShut: true, resizeDelta: 0};
    } else {
      {...model, width: newWidth, shouldSnapShut: true, resizeDelta: 0};
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
    {...state, shouldSnapShut: true, isOpen: !state.isOpen};
  } else {
    {...state, shouldSnapShut: true, isOpen: true, selected: pane};
  };

let setDefaultLocation = (state, setting) => {
  let location = setting == "right" ? Right : Left;
  {...state, location};
};

type settings = {
  sideBarLocation: string,
  sideBarVisibility: bool,
};

let setDefaults = (state, settings) => {
  let {sideBarVisibility, sideBarLocation} = settings;
  let state = setDefaultLocation(state, sideBarLocation);
  setDefaultVisibility(state, sideBarVisibility);
};
