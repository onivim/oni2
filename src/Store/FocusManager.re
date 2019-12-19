open Oni_Model;

let push = (focusable: Focus.focusable, state: State.t) =>
  switch (focusable) {
  | Quickmenu
  | Wildmenu =>
    failwith("Not allowed to push " ++ Focus.show_focusable(focusable))
  | _ => {...state, focus: Focus.push(focusable, state.focus)}
  };

let pop = (focusable: Focus.focusable, state: State.t) =>
  switch (focusable) {
  | Quickmenu
  | Wildmenu =>
    failwith("Not allowed to pop " ++ Focus.show_focusable(focusable))
  | _ => {...state, focus: Focus.pop(focusable, state.focus)}
  };

let current = (state: State.t) =>
  switch (state.quickmenu) {
  | Some({variant: Actions.Wildmenu(_)}) => Some(Focus.Wildmenu)
  | Some(_) => Some(Focus.Quickmenu)
  | _ => Focus.current(state.focus)
  };
