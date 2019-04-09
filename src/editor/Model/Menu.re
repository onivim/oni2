type t('a) = {
  searchQuery: string,
  isOpen: bool,
  commands: list(Actions.menuCommand('a)),
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
