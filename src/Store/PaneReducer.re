/*
 * PaneReducer.re
 */

open Oni_Model;

let reduce = (action: Actions.t, state: State.t) => {
  let { pane, _ }: State.t = state;
  let pt = Pane.getType(state.pane);

  let isSearchOpen = pt == Some(Pane.Search);
  let isSearchFocused = FocusManager.current(state) == Search;
  let isDiagnosticsOpen = pt == Some(Pane.Diagnostics);

  switch (action) {
  | ActivityBar(ActivityBar.SearchClick) when isSearchOpen && isSearchFocused =>
    {
    ...state,
    pane: Pane.setClosed(pane)
    };
  | ActivityBar(ActivityBar.SearchClick) when isSearchOpen && !isSearchFocused =>
    state |> FocusManager.push(Search);
  | ActivityBar(ActivityBar.SearchClick) when !isSearchOpen && !isSearchFocused =>
    { ...state, pane: Pane.setOpen(Pane.Search) }  |> FocusManager.push(Search);
  | StatusBar(StatusBarModel.DiagnosticsClicked) when isDiagnosticsOpen =>
    { ...state, pane: Pane.setClosed(pane) }
  | DiagnosticsHotKey when isDiagnosticsOpen =>
    { ...state, pane: Pane.setClosed(pane) }
  | StatusBar(StatusBarModel.DiagnosticsClicked) when !isDiagnosticsOpen =>
    { ...state, pane: Pane.setOpen(Pane.Diagnostics) }
  | DiagnosticsHotKey when !isDiagnosticsOpen =>
    { ...state, pane: Pane.setOpen(Pane.Diagnostics) }
  | PaneOpen(paneType) => { ...state, pane: Pane.setOpen(paneType) }
  | PaneClosed => { ...state, pane: Pane.setClosed(pane) }
  | _ => state
  };
};
