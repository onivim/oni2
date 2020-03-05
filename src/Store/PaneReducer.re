/*
 * PaneReducer.re
 */

open Oni_Model;
open Actions;

let showSearchPane = (state: State.t, isSearchFocused) => {
  let newState = {
    ...state,
    pane: {
      isOpen: true,
      selected: Search,
    },
  };

  if (isSearchFocused) {
    newState;
  } else {
    newState |> FocusManager.push(Search);
  };
};

let hideSearchPane = (state: State.t, isSearchFocused) => {
  let newState = {
    ...state,
    pane: {
      ...state.pane,
      isOpen: false,
    },
  };

  if (isSearchFocused) {
    newState |> FocusManager.pop(Search);
  } else {
    newState;
  };
};

let openDiagnosticsPane = (state: State.t) => {
  ...state,
  pane: {
    isOpen: true,
    selected: Diagnostics,
  },
};

let openNotificationsPane = (state: State.t) => {
  ...state,
  pane: {
    isOpen: true,
    selected: Notifications,
  },
};

let closePane = (state: State.t) => {
  ...state,
  pane: {
    ...state.pane,
    isOpen: false,
  },
};

let reduce = (action: Actions.t, state: State.t) =>
  switch (action) {
  | SearchHotkey
  | ActivityBar(SearchClick) when Pane.isVisible(Search, state.pane) =>
    FocusManager.current(state) == Search
      ? hideSearchPane(state, true) : showSearchPane(state, false)

  | SearchHotkey
  | PaneTabClicked(Search)
  | ActivityBar(SearchClick) =>
    showSearchPane(state, FocusManager.current(state) == Search)

  | DiagnosticsHotKey
  | StatusBar(DiagnosticsClicked)
      when Pane.isVisible(Diagnostics, state.pane) =>
    closePane(state)

  | DiagnosticsHotKey
  | PaneTabClicked(Diagnostics)
  | StatusBar(DiagnosticsClicked) => openDiagnosticsPane(state)

  | StatusBar(NotificationCountClicked)
      when Pane.isVisible(Notifications, state.pane) =>
    closePane(state)

  | PaneTabClicked(Notifications)
  | StatusBar(NotificationCountClicked) => openNotificationsPane(state)

  | PaneCloseButtonClicked => closePane(state)

  | _ => state
  };
