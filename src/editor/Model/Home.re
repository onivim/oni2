type guiTab = {
  title: string,
  onClick: unit => Actions.t,
  onClose: unit => Actions.t,
  selected: bool,
};

type t = {tabs: list(guiTab)};

let createTabs = () => [
  {
    title: "Welcome to Oni2",
    selected: true,
    onClose: () => CloseHome,
    onClick: () => OpenHome,
  },
];

let create = () => {tabs: createTabs()};

let reduce = (state: t, action: Actions.t) =>
  switch (action) {
  | OpenHome => {tabs: createTabs()}
  | CloseHome => {tabs: []}
  | _ => state
  };
