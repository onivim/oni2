type guiTab = {
  title: string,
  onClick: unit => Actions.t,
  onClose: unit => Actions.t,
};

type t = {
  isOpen: bool,
  tabs: list(guiTab),
};

let create: unit => t;

let reduce: (t, Actions.t) => t;
