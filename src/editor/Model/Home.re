type guiTab = {
  title: string,
  onClick: unit => Actions.t,
  onClose: unit => Actions.t,
};

type t = {
  isOpen: bool,
  tabs: list(guiTab),
};

let reduce = (state: t, action: Actions.t) =>
  switch (action) {
  | ShowHome => {...state, isOpen: true}
  | ShowEditor => {...state, isOpen: false}
  /* | BufferEnter(_) => {...state, isOpen: false} */
  | _ => state
  };

let noop = () => ();

let createTabs = () => [
  {
    title: "Welcome to Oni2",
    onClose: () => ShowEditor,
    onClick: () => ShowHome,
  },
];

let create = () => {isOpen: true, tabs: createTabs()};
