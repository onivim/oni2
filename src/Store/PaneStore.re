/*
 * PaneReducer.re
 */

open Oni_Model;
open Actions;
open Feature_Pane;

let focus = (state: State.t) =>
  if (Feature_Pane.isOpen(state.pane)
      && Feature_Pane.isVisible(Search, state.pane)) {
    FocusManager.push(Search, state);
  } else {
    state;
  };

let showSearchPane = (state: State.t) =>
  {...state, pane: Feature_Pane.show(~pane=Search, state.pane)}
  |> FocusManager.push(Search);

let hideSearchPane = (state: State.t) =>
  {...state, pane: Feature_Pane.close(state.pane)}
  |> FocusManager.pop(Search);

let openDiagnosticsPane = (state: State.t) => {
  ...state,
  pane: Feature_Pane.show(~pane=Diagnostics, state.pane),
};

let openNotificationsPane = (state: State.t) => {
  ...state,
  pane: Feature_Pane.show(~pane=Notifications, state.pane),
};

let closePane = (state: State.t) =>
  {...state, pane: Feature_Pane.close(state.pane)}
  // TODO: Generalize this
  |> FocusManager.pop(Search);

let update = (state: State.t, action: Actions.t) =>
  switch (action) {
  | SearchHotkey
  | ActivityBar(SearchClick) when Feature_Pane.isVisible(Search, state.pane) => (
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
      when Feature_Pane.isVisible(Diagnostics, state.pane) => (
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
      when Feature_Pane.isVisible(Notifications, state.pane) => (
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
