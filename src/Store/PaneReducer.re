/*
 * PaneReducer.re
 */

open Oni_Model;

let reduce = (state: Pane.t, action: Actions.t) => {
  switch (action) {
  | PaneOpen(paneType) => Pane.setOpen(paneType)
  | PaneClosed => Pane.setClosed(state)
  | _ => state
  };
};
