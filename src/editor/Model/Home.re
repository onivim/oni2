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

let create = () => {tabs: []};

let deselectTabs = (tabs: list(guiTab)) =>
  List.map(tab => {...tab, selected: false}, tabs);

let reduce = (state: t, action: Actions.t) =>
  switch (action) {
  | BufferEnter(_) => {tabs: deselectTabs(state.tabs)}
  | OpenHome => {tabs: createTabs()}
  | CloseHome => {tabs: []}
  | _ => state
  };
