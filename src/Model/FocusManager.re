open Oni_Core.Utility;

let push = (focusable: Focus.focusable, state: State.t) =>
  switch (focusable) {
  | Sneak
  | Modal
  | Quickmenu
  | Wildmenu =>
    failwith("Not allowed to push " ++ Focus.show_focusable(focusable))
  | _ => {...state, focus: Focus.push(focusable, state.focus)}
  };

let pop = (focusable: Focus.focusable, state: State.t) =>
  switch (focusable) {
  | Sneak
  | Modal
  | Quickmenu
  | Wildmenu =>
    failwith("Not allowed to pop " ++ Focus.show_focusable(focusable))
  | _ => {...state, focus: Focus.pop(focusable, state.focus)}
  };

let current = (state: State.t) =>
  if (Feature_Sneak.isActive(state.sneak)) {
    Focus.Sneak;
  } else if (Feature_Registers.isActive(state.registers)) {
    Focus.InsertRegister;
  } else if (state.modal != None) {
    Focus.Modal;
  } else {
    switch (state.quickmenu) {
    | Some({variant: Actions.Wildmenu(_), _}) => Focus.Wildmenu
    | Some(_) => Focus.Quickmenu
    | _ =>
      state
      // See if terminal has focus
      |> Selectors.getActiveBuffer
      |> Option.map(Oni_Core.Buffer.getId)
      |> Option.map(id => BufferRenderers.getById(id, state.bufferRenderers))
      |> OptionEx.flatMap(renderer =>
           switch (renderer) {
           | BufferRenderer.Terminal({id, insertMode, _}) when insertMode =>
             Some(Focus.Terminal(id))
           | _ => None
           }
         )
      // Otherwise, get from assigned focus state
      |> OptionEx.or_(Focus.current(state.focus))
      |> Option.value(~default=Focus.Editor)
    };
  };
