open Oni_Core;

type t = {
  searchQuery: string,
  isOpen: bool,
  commands: list(Actions.menuCommand),
  selectedItem: int,
  filterJob: MenuJob.t,
  dispose: unit => unit,
};

let create = () => {
  searchQuery: "",
  isOpen: false,
  commands: [],
  selectedItem: 0,
  filterJob: MenuJob.create(),
  dispose: () => (),
};
