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

let update = (state: State.t, action: Actions.t) =>
  switch (action) {
  | SearchHotkey
  | ActivityBar(SearchClick) when Pane.isVisible(Search, state.pane) => (
      FocusManager.current(state) == Search
        ? hideSearchPane(state, true) : showSearchPane(state, false),
      Isolinear.Effect.none,
    )

  | SearchHotkey
  | PaneTabClicked(Search)
  | ActivityBar(SearchClick) => (
      showSearchPane(state, FocusManager.current(state) == Search),
      Isolinear.Effect.none,
    )

  | DiagnosticsHotKey
  | StatusBar(DiagnosticsClicked)
      when Pane.isVisible(Diagnostics, state.pane) => (
      closePane(state),
      Isolinear.Effect.none,
    )

  | DiagnosticsHotKey
  | PaneTabClicked(Diagnostics)
  | StatusBar(DiagnosticsClicked) => (
      openDiagnosticsPane(state),
      Isolinear.Effect.none,
    )

  | StatusBar(NotificationCountClicked)
      when Pane.isVisible(Notifications, state.pane) => (
      closePane(state),
      Isolinear.Effect.none,
    )

  | PaneTabClicked(Notifications)
  | StatusBar(NotificationCountClicked) => (
      openNotificationsPane(state),
      Isolinear.Effect.none,
    )

  | StatusBar(NotificationClearAllClicked) => (
      state,
      Feature_Notification.Effects.dismissAll
      |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
    )

  | PaneCloseButtonClicked => (closePane(state), Isolinear.Effect.none)

  | _ => (state, Isolinear.Effect.none)
  };
