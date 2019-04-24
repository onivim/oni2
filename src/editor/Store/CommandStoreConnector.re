open Oni_Core;

open Oni_Model;
open Oni_Model.Actions;

let start = () => {

  let singleActionEffect = (action, name) => Isolinear.Effect.createWithDispatch(~name="command." ++ name, dispatch => dispatch(action));
  let multipleActionEffect = (actions, name) => Isolinear.Effect.createWithDispatch(~name="command." ++ name, dispatch => List.iter(v => dispatch(v), actions));


  let commands = [
   ("commandPalette.open", _ => multipleActionEffect([MenuOpen(CommandPalette.create), SetInputControlMode(TextInputFocus)])),
    ("quickOpen.open", _ => singleActionEffect(QuickOpen)),
    ("menu.open", _ => multipleActionEffect([MenuClose, SetInputControlMode(EditorTextFocus)])),
    ("menu.next", _ => multipleActionEffect([SetInputControlMode(MenuFocus), MenuNextItem])),
    ("menu.previous", _ => multipleActionEffect([SetInputControlMode(MenuFocus), MenuPreviousItem])),
    ("menu.select", _ => multipleActionEffect([MenuSelect, SetInputControlMode(EditorTextFocus)])),
  ];

  let commandMap = List.fold_left((prev, curr) => {
      let (command, handler) = curr;
      StringMap.add(command, handler, prev);
  }, StringMap.empty, commands);
  

  let updater = (state: State.t, action) => {
    switch (action) {
    | Command(cmd) => switch (StringMap.find_opt(cmd, commandMap)) {
    | Some(v) => (state, v(state, cmd))
    | None =>  (state, Isolinear.Effect.none)
    }
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
