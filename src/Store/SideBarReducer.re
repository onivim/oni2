/*
 * SideBarReducer.re
 */

open Oni_Model;
open Actions;

open SideBar;

let reduce = (state: SideBar.t, action: Actions.t) => {
  switch (action) {
  | ActivityBar(ActivityBar.FileExplorerClick) => SideBar.toggle(SideBar.FileExplorer, state)
  | ActivityBar(ActivityBar.ExtensionsClick) => 
    SideBar.toggle(SideBar.Extensions, state)
  | _ => state;
  };
};
