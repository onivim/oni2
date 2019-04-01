type guiTab = {
  title: string,
  onClick: unit => Actions.t,
  onClose: unit => Actions.t,
};

type t = {
  isOpen: bool,
  tabs: list(guiTab),
};

let createTabs = () => [
  {
    title: "Welcome to Oni2",
    onClose: () => ShowEditor,
    onClick: () => ShowHome,
  },
];

let create = () => {isOpen: true, tabs: createTabs()};

let reduce = (state: t, action: Actions.t) =>
  switch (action) {
  | ShowHome => {isOpen: true, tabs: createTabs()}
  | ShowEditor => {isOpen: false, tabs: []}
  | _ => state
  };
