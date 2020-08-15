/*
 * SideBarReducer.re
 */

open Oni_Core;
open Oni_Model;
open Actions;

let focus = (state: State.t) =>
  if (state.sideBar |> Feature_SideBar.isOpen) {
    switch (state.sideBar |> Feature_SideBar.selected) {
    | FileExplorer => FocusManager.push(FileExplorer, state)
    | SCM => FocusManager.push(SCM, state)
    | _ => state
    };
  } else {
    state;
  };

let reduce = (~zenMode, state: Feature_SideBar.model, action: Actions.t) => {
  switch (action) {
  // When we're in Zen mode, we ignore toggling, and exit zen mode
  | ActivityBar(ActivityBar.FileExplorerClick) when !zenMode =>
    Feature_SideBar.toggle(Feature_SideBar.FileExplorer, state)
  | ActivityBar(ActivityBar.SCMClick) when !zenMode =>
    Feature_SideBar.toggle(Feature_SideBar.SCM, state)
  | ActivityBar(ActivityBar.ExtensionsClick) when !zenMode =>
    Feature_SideBar.toggle(Feature_SideBar.Extensions, state)
  | ConfigurationSet(newConfig) =>
    let sideBarLocation =
      Configuration.getValue(c => c.workbenchSideBarLocation, newConfig);
    let sideBarVisibility =
      Configuration.getValue(c => c.workbenchSideBarVisible, newConfig);
    Feature_SideBar.setDefaults(state, {sideBarLocation, sideBarVisibility});
  | _ => state
  };
};
