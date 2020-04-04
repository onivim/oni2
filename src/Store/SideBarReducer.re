/*
 * SideBarReducer.re
 */

open Oni_Core;
open Oni_Model;
open Actions;

let reduce = (state: SideBar.t, action: Actions.t) => {
  switch (action) {
  | ActivityBar(ActivityBar.FileExplorerClick) =>
    SideBar.toggle(SideBar.FileExplorer, state)
  | ActivityBar(ActivityBar.SCMClick) => SideBar.toggle(SideBar.SCM, state)
  | ActivityBar(ActivityBar.ExtensionsClick) =>
    SideBar.toggle(SideBar.Extensions, state)
  | ConfigurationSet(newConfig) =>
    let sideBarSetting =
      Configuration.getValue(c => c.workbenchSideBarVisible, newConfig);
    SideBar.setDefaultVisibility(state, sideBarSetting);
  | _ => state
  };
};
