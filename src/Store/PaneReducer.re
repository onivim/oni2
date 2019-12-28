/*
 * PaneReducer.re
 */

open Oni_Model;

let reduce = (state: Pane.t, action: Actions.t) => {
  let pt = Pane.getType(state);

  switch (action) {
  | ActivityBar(ActivityBar.SearchClick) when pt == Some(Pane.Search) =>
    Pane.setClosed(state)
  | ActivityBar(ActivityBar.SearchClick) when pt != Some(Pane.Search) =>
    Pane.setOpen(Pane.Search)
  | DiagnosticsHotKey when pt == Some(Pane.Diagnostics) =>
    Pane.setClosed(state)
  | DiagnosticsHotKey when pt !== Some(Pane.Diagnostics) =>
    Pane.setOpen(Pane.Diagnostics)
  | StatusBar(StatusBarModel.DiagnosticsClicked)
      when pt == Some(Pane.Diagnostics) =>
    Pane.setClosed(state)
  | StatusBar(StatusBarModel.DiagnosticsClicked)
      when pt !== Some(Pane.Diagnostics) =>
    Pane.setOpen(Pane.Diagnostics)
  | PaneOpen(paneType) => Pane.setOpen(paneType)
  | PaneClosed => Pane.setClosed(state)
  | _ => state
  };
};
