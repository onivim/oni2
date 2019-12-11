/*
 * PaneReducer.re
 */

open Oni_Model;

let reduce = (state: Pane.t, action: Actions.t) => {
  switch (action) {
  | PaneShow(paneType) => Pane.show(paneType)
  | PaneHide => Pane.hide(state)
  | _ => state
  };
};
