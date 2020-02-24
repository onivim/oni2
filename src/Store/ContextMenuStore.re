open Oni_Model;
open Actions;

let start = () => {
  let selectItemEffect = (item: ContextMenu.item(_)) =>
    Isolinear.Effect.createWithDispatch(
      ~name="contextMenu.selectItem", dispatch =>
      dispatch(item.data)
    );

  let updater = (state: State.t, action) => {
    let default = (state, Isolinear.Effect.none);

    switch (action) {
    | ContextMenuOverlayClicked => (
        {...state, contextMenu: State.ContextMenu.Nothing},
        Isolinear.Effect.none,
      )

    | ContextMenuItemSelected(item) => (
        {...state, contextMenu: State.ContextMenu.Nothing},
        selectItemEffect(item),
      )

    | StatusBar(NotificationsContextMenu) => (
        {...state, contextMenu: State.ContextMenu.NotificationStatusBarItem},
        Isolinear.Effect.none,
      )

    | _ => default
    };
  };

  updater;
};
