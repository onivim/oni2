/*
 * StatusBarReducer
 *
 * State changes for status bar
 */

open StatusBarModel.Item;

let reduce = (state: StatusBarModel.t, action: Actions.t(State.t)) => {
  let removeItemById = (items: StatusBarModel.t, id) => {
    List.filter(si => si.id !== id, items);
  };

  switch (action) {
  | StatusBarAddItem(item) =>
    /* Replace the old item with the new one */
    let newState = removeItemById(state, item.id);
    [item, ...newState];
  | StatusBarDisposeItem(id) => removeItemById(state, id)
  | _ => state
  };
};
