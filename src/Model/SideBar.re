type pane =
  | FileExplorer
  | SCM
  | Extensions;

type t = {
  // Track the last value of 'workbench.sideBar.visible'
  // If it changes, we should update
  openByDefault: bool,
  isOpen: bool,
  selected: pane,
};

let initial = {openByDefault: false, isOpen: false, selected: FileExplorer};

let isVisible = (pane, model) => model.isOpen && model.selected == pane;

let setDefaultVisibility = (pane, defaultVisibility) =>
  if (pane.openByDefault == defaultVisibility) {
    pane;
  } else {
    {...pane, openByDefault: defaultVisibility, isOpen: defaultVisibility};
  };

let toggle = (pane, state: t) =>
  if (pane == state.selected) {
    {...state, isOpen: !state.isOpen};
  } else {
    {...state, isOpen: true, selected: pane};
  };
