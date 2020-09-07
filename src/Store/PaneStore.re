/*
 * PaneReducer.re
 */

open Oni_Model;
open Actions;
open Feature_Pane;

let openDiagnosticsPane = (state: State.t) => {
  ...state,
  pane: Feature_Pane.show(~pane=Diagnostics, state.pane),
};

let openNotificationsPane = (state: State.t) => {
  ...state,
  pane: Feature_Pane.show(~pane=Notifications, state.pane),
};

let closePane = (state: State.t) => {
  ...state,
  pane: Feature_Pane.close(state.pane),
};

let update = (state: State.t, action: Actions.t) =>
  switch (action) {
  //  | DiagnosticsHotKey
  //  | StatusBar(Feature_StatusBar.DiagnosticsClicked)
  //      when Feature_Pane.isVisible(Diagnostics, state.pane) => (
  //      closePane(state),
  //      Isolinear.Effect.none,
  //    )

  //  | DiagnosticsHotKey
  //  | PaneTabClicked(Diagnostics)
  //  | StatusBar(Feature_StatusBar.DiagnosticsClicked) => (
  //      openDiagnosticsPane(state),
  //      Isolinear.Effect.none,
  //    )
  //  | StatusBar(Feature_StatusBar.NotificationCountClicked)
  //      when Feature_Pane.isVisible(Notifications, state.pane) => (
  //      closePane(state),
  //      Isolinear.Effect.none,
  //    )
  //  | PaneTabClicked(Notifications)
  //  | StatusBar(Feature_StatusBar.ContextMenuNotificationOpenClicked)
  //  | StatusBar(Feature_StatusBar.NotificationCountClicked) => (
  //      openNotificationsPane(state),
  //      Isolinear.Effect.none,
  //    )
  //  | StatusBar(Feature_StatusBar.ContextMenuNotificationClearAllClicked) => (
  //      state,
  //      Feature_Notification.Effects.dismissAll
  //      |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
  //    )
  //  | PaneCloseButtonClicked => (closePane(state), Isolinear.Effect.none)

  | _ => (state, Isolinear.Effect.none)
  };
