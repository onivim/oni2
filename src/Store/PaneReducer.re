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

let closePane = (state: State.t) => {
  ...state,
  pane: Pane.setClosed(state.pane),
};

let openDiagnosticsPane = (state: State.t) => {
  ...state,
  pane: Pane.setOpen(Pane.Diagnostics),
};

let openNotificationsPane = (state: State.t) => {
  ...state,
  pane: Pane.setOpen(Pane.Notifications),
};

let reduce = (action: Actions.t, state: State.t) =>
  switch (action, Pane.getType(state.pane)) {
  | (SearchHotkey, Some(Pane.Search))
  | (ActivityBar(ActivityBar.SearchClick), Some(Pane.Search)) =>
    FocusManager.current(state) == Search
      ? hideSearchPane(state, true) : showSearchPane(state, false)

  | (SearchHotkey, _)
  | (PaneTabClicked(Pane.Search), _)
  | (ActivityBar(ActivityBar.SearchClick), _) =>
    showSearchPane(state, FocusManager.current(state) == Search)

  | (DiagnosticsHotKey, Some(Pane.Diagnostics))
  | (StatusBar(StatusBarModel.DiagnosticsClicked), Some(Pane.Diagnostics)) =>
    closePane(state)

  | (DiagnosticsHotKey, _)
  | (PaneTabClicked(Pane.Diagnostics), _)
  | (StatusBar(StatusBarModel.DiagnosticsClicked), _) =>
    openDiagnosticsPane(state)

  | (
      StatusBar(StatusBarModel.NotificationCountClicked),
      Some(Pane.Notifications),
    ) =>
    closePane(state)

  | (PaneTabClicked(Pane.Notifications), _)
  | (StatusBar(StatusBarModel.NotificationCountClicked), _) =>
    openNotificationsPane(state)

  | _ => state
  };
