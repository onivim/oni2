module Event = Rench.Event;

type t = {
  searchQuery: string,
  isOpen: bool,
  /*
     [isLoading] - whether or not the menu is waiting on some asynchronous actions.
     As an example, for QuickOpen, the [isLoading] flag would be [true] while the
     ripgrep process is active. This is used to give a visual indication that the
     menu is still populating.
   */
  isLoading: bool,
  selectedItem: int,
  filterJob: MenuJob.t,
  onQueryChanged: Event.t(string),
  dispose: unit => unit,
};

let create = () => {
  searchQuery: "",
  isOpen: false,
  isLoading: false,
  selectedItem: 0,
  filterJob: MenuJob.create(),
  onQueryChanged: Event.create(),
  dispose: () => (),
};
