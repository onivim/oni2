open Oni_Model;
open Actions;

module ContextMenu = Oni_Components.ContextMenu;

let start = () => {
  let selectItemEffect = (item: ContextMenu.item(_)) =>
    Isolinear.Effect.createWithDispatch("contextMenu.selectItem", dispatch =>
      dispatch(item.data)
    );

  let updater = (state: State.t, action) => {
    let default = (state, Isolinear.Effect.none);

    switch (action) {
    | ContextMenuUpdate(model) => (
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

    | _ => default
    };
  };

  updater;
};
