/*
 * SideBarReducer.re
 */

open Oni_Core;
open Oni_Model;
open Actions;

let reduce = (state: Feature_SideBar.model, action: Actions.t) => {
  switch (action) {
  // When we're in Zen mode, we ignore toggling, and exit zen mode
  | ConfigurationSet(newConfig) =>
    let sideBarLocation =
      Configuration.getValue(c => c.workbenchSideBarLocation, newConfig);
    let sideBarVisibility =
      Configuration.getValue(c => c.workbenchSideBarVisible, newConfig);
    Feature_SideBar.setDefaults(state, {sideBarLocation, sideBarVisibility});
  | _ => state
  };
};
