type pane =
  | FileExplorer
  | SCM
  | Extensions;

type model = {
  // Track the last value of 'workbench.sideBar.visible'
  // If it changes, we should update
  openByDefault: bool,
  isOpen: bool,
  selected: pane,
};

let selected = ({selected, _}) => selected;
let isOpen = ({isOpen, _}) => isOpen;

let initial = {openByDefault: false, isOpen: false, selected: FileExplorer};

let isVisible = (pane, model) => model.isOpen && model.selected == pane;

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
