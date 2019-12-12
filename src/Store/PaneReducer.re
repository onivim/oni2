/*
 * PaneReducer.re
 */

open Oni_Model;

let reduce = (state: Pane.t, action: Actions.t) => {
  switch (action) {
  | PaneVisible(paneType) => Pane.setOpen(paneType)
  | PaneHiddene => Pane.hide(state)
  | _ => state
  };
};
