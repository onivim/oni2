/*
 * StatusBarReducer
 *
 * State changes for status bar
 */

open StatusBar.Item;

let reduce = (state: StatusBar.t, action: Actions.t) => {
  let removeItemById = (items: StatusBar.t, id) => {
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
