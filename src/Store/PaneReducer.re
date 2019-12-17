/*
 * PaneReducer.re
 */

open Oni_Model;

let reduce = (state: Pane.t, action: Actions.t) => {

  let pt = Pane.getType(state);

  switch (action) {
  | ActivityBar(ActivityBar.SearchClick) when pt == Some(Pane.Search) => 
    Pane.setClosed(state)
  | ActivityBar(ActivityBar.SearchClick) when pt != Some(Pane.Search) => Pane.setOpen(Pane.Search)
  | PaneOpen(paneType) => Pane.setOpen(paneType)
  | PaneClosed => Pane.setClosed(state)
  | _ => state
  };
};
