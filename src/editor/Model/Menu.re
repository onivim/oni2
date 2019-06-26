type t = {
  searchQuery: string,
  isOpen: bool,
  commands: list(Actions.menuCommand),
  filteredCommands: list(Actions.menuCommand),
  selectedItem: int,
  dispose: unit => unit,
};

let create = () => {
  searchQuery: "",
  isOpen: false,
  commands: [],
  filteredCommands: [],
  selectedItem: 0,
  dispose: () => (),
};
