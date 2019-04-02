type t = {
  searchQuery: string,
  isOpen: bool,
  commands: list(MenuCommand.t),
  selectedItem: int,
  dispose: unit => unit,
};

let create = () => {
  searchQuery: "",
  isOpen: false,
  commands: [],
  selectedItem: 0,
  dispose: () => (),
};
