/*
 * PaneReducer.re
 */

open Oni_Model;
open Actions;
let showSearchPane = (state: State.t, isSearchFocused) => {
  let newState = {...state, pane: Pane.setOpen(Pane.Search)};

  if (isSearchFocused) {
    newState;
  } else {
    newState |> FocusManager.push(Search);
  };
};

let hideSearchPane = (state: State.t, isSearchFocused) => {
  let newState = {...state, pane: Pane.setClosed(state.pane)};

  if (isSearchFocused) {
    newState |> FocusManager.pop(Search);
  } else {
    newState;
  };
};

let searchPaneReducer = (action, state: State.t) => {
  let paneType = Pane.getType(state.pane);

  let isSearchOpen = paneType == Some(Pane.Search);
  let isSearchFocused = FocusManager.current(state) == Search;

  switch (action, isSearchOpen, isSearchFocused) {
  // Search pane open, and focused
  | (SearchHotkey, true, true)
  | (ActivityBar(ActivityBar.SearchClick), true, true) =>
    hideSearchPane(state, true)

  // Search pane not open, not focused
  | (SearchHotkey, true, false)
  | (ActivityBar(ActivityBar.SearchClick), true, false) =>
    showSearchPane(state, false)

  | (SearchHotkey, false, _)
  | (ActivityBar(ActivityBar.SearchClick), false, _) =>
    showSearchPane(state, isSearchFocused)
  | _ => state
  };
};

let closeDiagnosticsPane = (state: State.t) => {
  ...state,
  pane: Pane.setClosed(state.pane),
};

let openDiagnosticsPane = (state: State.t) => {
  ...state,
  pane: Pane.setOpen(Pane.Diagnostics),
};

let diagnosticsPaneReducer = (action, state: State.t) => {
  let isDiagnosticsOpen = Pane.getType(state.pane) == Some(Pane.Diagnostics);

  switch (action, isDiagnosticsOpen) {
  // Diagnostics already open
  | (StatusBar(StatusBarModel.DiagnosticsClicked), true)
  | (DiagnosticsHotKey, true) => closeDiagnosticsPane(state)
  | (StatusBar(StatusBarModel.DiagnosticsClicked), false)
  | (DiagnosticsHotKey, false) => openDiagnosticsPane(state)
  | _ => state
  };
};

let reduce = (action: Actions.t, state: State.t) => {
  state |> searchPaneReducer(action) |> diagnosticsPaneReducer(action);
};
