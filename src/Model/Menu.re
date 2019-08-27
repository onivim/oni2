module Event = Rench.Event;

type t = {
  searchQuery: string,
  isOpen: bool,
  creationTime: float,
  /*
     [isLoading] - whether or not the menu is waiting on some asynchronous actions.
     As an example, for QuickOpen, the [isLoading] flag would be [true] while the
     ripgrep process is active. This is used to give a visual indication that the
     menu is still populating.
   */
  isLoading: bool,
  loadingAnimation: Animation.t,
  selectedItem: int,
  rowOffset: int,
  filterJob: MenuJob.t,
  onQueryChanged: Event.t(string),
  dispose: unit => unit,
  maxRows: int,
  rowHeight: int,
  scrollY: float,
};

let create = () => {
  creationTime: 0.,
  loadingAnimation: Animation.create(~isActive=false, ~duration=2., ()),
  searchQuery: "",
  isOpen: false,
  isLoading: false,
  selectedItem: 0,
  rowOffset: 0,
  maxRows: 8,
  filterJob: MenuJob.create(),
  onQueryChanged: Event.create(),
  dispose: () => (),
  rowHeight: 40,
  scrollY: 0.0,
};
