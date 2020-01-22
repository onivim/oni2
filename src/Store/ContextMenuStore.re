open Oni_Model;
open Actions;

module ContextMenu = Oni_Components.ContextMenu;

let contextMenu =
  Notifications.ContextMenu.init(
    ContextMenu.[
      {
        label: "Clear All",
        // icon: None,
        data: ClearNotifications,
      },
      {
        label: "Open",
        // icon: None,
        data: StatusBar(NotificationCountClicked),
      },
    ],
  );

let start = () => {
  let selectItemEffect = (item: ContextMenu.item(_)) =>
    Isolinear.Effect.createWithDispatch(
      ~name="contextMenu.selectItem", dispatch =>
      dispatch(item.data)
    );

  let updater = (state: State.t, action) => {
    let default = (state, Isolinear.Effect.none);

    switch (action) {
    | ContextMenuUpdated(model) => (
        {...state, contextMenu: Some(model)},
        Isolinear.Effect.none,
      )

    | ContextMenuOverlayClicked => (
        {...state, contextMenu: None},
        Isolinear.Effect.none,
      )

    | ContextMenuItemSelected(item) => (
        {...state, contextMenu: None},
        selectItemEffect(item),
      )

    | StatusBar(NotificationsContextMenu) => (
        {...state, contextMenu: Some(contextMenu)},
        Isolinear.Effect.none,
      )

    | _ => default
    };
  };

  updater;
};
