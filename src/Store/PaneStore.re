/*
 * PaneReducer.re
 */

open Oni_Model;
open Actions;

let showSearchPane = (state: State.t) =>
  {
    ...state,
    pane: {
      isOpen: true,
      selected: Search,
    },
  }
  |> FocusManager.push(Search);

let hideSearchPane = (state: State.t) =>
  {
    ...state,
    pane: {
      ...state.pane,
      isOpen: false,
    },
  }
  |> FocusManager.pop(Search);

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

let closePane = (state: State.t) =>
  {
    ...state,
    pane: {
      ...state.pane,
      isOpen: false,
    },
  }
  |> FocusManager.pop(Search);

let update = (state: State.t, action: Actions.t) =>
  switch (action) {
  | SearchHotkey
  | ActivityBar(SearchClick) when Pane.isVisible(Search, state.pane) => (
      FocusManager.current(state) == Search
        ? hideSearchPane(state) : showSearchPane(state),
      Isolinear.Effect.none,
    )

  | SearchHotkey
  | PaneTabClicked(Search)
  | ActivityBar(SearchClick) => (
      showSearchPane(state),
      Isolinear.Effect.none,
    )

  | DiagnosticsHotKey
  | StatusBar(Feature_StatusBar.DiagnosticsClicked)
      when Pane.isVisible(Diagnostics, state.pane) => (
      closePane(state),
      Isolinear.Effect.none,
    )

  | DiagnosticsHotKey
  | PaneTabClicked(Diagnostics)
  | StatusBar(Feature_StatusBar.DiagnosticsClicked) => (
      openDiagnosticsPane(state),
      Isolinear.Effect.none,
    )

  | StatusBar(Feature_StatusBar.NotificationCountClicked)
      when Pane.isVisible(Notifications, state.pane) => (
      closePane(state),
      Isolinear.Effect.none,
    )

  | PaneTabClicked(Notifications)
  | StatusBar(Feature_StatusBar.ContextMenuNotificationOpenClicked)
  | StatusBar(Feature_StatusBar.NotificationCountClicked) => (
      openNotificationsPane(state),
      Isolinear.Effect.none,
    )

  | StatusBar(Feature_StatusBar.ContextMenuNotificationClearAllClicked) => (
      state,
      Feature_Notification.Effects.dismissAll
      |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
    )

  | PaneCloseButtonClicked => (closePane(state), Isolinear.Effect.none)

  | _ => (state, Isolinear.Effect.none)
  };
