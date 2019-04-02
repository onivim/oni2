type t = {
  searchQuery: string,
  isOpen: bool,
  commands: list(MenuCommand.t),
  selectedItem: int,
};

let create = () => {
  searchQuery: "",
  isOpen: false,
  commands: [],
  selectedItem: 0,
};
