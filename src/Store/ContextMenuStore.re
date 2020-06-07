open Oni_Model;
open Actions;

let start = () => {
  let updater = (state: State.t, action) => {
    let default = (state, Isolinear.Effect.none);

    switch (action) {
    | ContextMenuOverlayClicked => (
        {...state, contextMenu: Feature_ContextMenu.Nothing},
        Isolinear.Effect.none,
      )

    | StatusBar(NotificationsCountRightClicked) => (
        {
          ...state,
          contextMenu: Feature_ContextMenu.NotificationStatusBarItem,
        },
        Isolinear.Effect.none,
      )
    | StatusBar(ContextMenuNotificationClearAllClicked)
    | StatusBar(ContextMenuNotificationOpenClicked) => (
        {...state, contextMenu: Feature_ContextMenu.Nothing},
        Isolinear.Effect.none,
      )

    | _ => default
    };
  };

  updater;
};
