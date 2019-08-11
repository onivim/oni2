module Event = Rench.Event;

type t = {
  searchQuery: string,
  isOpen: bool,
  commands: list(Actions.menuCommand),
  selectedItem: int,
  filterJob: MenuJob.t,
  onQueryChanged: Event.t(string),
  dispose: unit => unit,
};

let create = () => {
  searchQuery: "",
  isOpen: false,
  commands: [],
  selectedItem: 0,
  filterJob: MenuJob.create(),
  onQueryChanged: Event.create(),
  dispose: () => (),
};
