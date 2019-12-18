type sideBarType =
  | FileExplorer
  | Extensions;

type t = {
  isOpen: bool,
  activeType: sideBarType,
};

let initial = {isOpen: true, activeType: FileExplorer};

let setOpen = sideBarType => {isOpen: true, activeType: sideBarType};

let setClosed = (sideBar: t) => {...sideBar, isOpen: false};

let getType = sideBar => sideBar.activeType;

let isOpen = sideBar => sideBar.isOpen;

let toggle = (sideBarType, state: t) =>
  if (sideBarType == state.activeType) {
    {...state, isOpen: !state.isOpen};
  } else {
    {isOpen: true, activeType: sideBarType};
  };
