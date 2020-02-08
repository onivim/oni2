type pane =
  | FileExplorer
  | SCM
  | Extensions;

type t = {
  isOpen: bool,
  selected: pane,
};

let initial = {isOpen: true, selected: FileExplorer};

let isVisible = (pane, model) => model.isOpen && model.selected == pane;

let toggle = (pane, state: t) =>
  if (pane == state.selected) {
    {...state, isOpen: !state.isOpen};
  } else {
    {isOpen: true, selected: pane};
  };
