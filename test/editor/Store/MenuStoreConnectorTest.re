module MenuStore = Oni_Store__MenuStoreConnector;
module Model = Oni_Model;
open TestFramework;

let cmd: Model.Actions.menuCommand = {
  category: None,
  name: "command",
  command: () => Model.Actions.Init,
  icon: None,
};

let initial_state = Model.State.create();
let get_state = menu => {...initial_state, menu};

let commands = [cmd, cmd, cmd, cmd];

let onStateChange = MenuStore.start() |> fst |> Helpers.onStateChange;

let store =
  describe("MenuStoreConnector", ({describe, _}) => {
    describe("MenuPreviousItem", ({test, _}) => {
      test("goes from 1st to last", ({expect}) =>
        onStateChange(
          get_state({...initial_state.menu, commands}),
          Model.Actions.MenuPreviousItem,
          state =>
          expect.int(state.menu.selectedItem).toBe(3)
        )
      );

      test("goes from 2nd to 1st", ({expect}) =>
        onStateChange(
          get_state({...initial_state.menu, commands, selectedItem: 1}),
          Model.Actions.MenuPreviousItem,
          state =>
          expect.int(state.menu.selectedItem).toBe(0)
        )
      );
    });

    describe("MenuNextItem", ({test, _}) => {
      test("goes from last to 1st", ({expect}) =>
        onStateChange(
          get_state({...initial_state.menu, commands, selectedItem: 3}),
          Model.Actions.MenuNextItem,
          state =>
          expect.int(state.menu.selectedItem).toBe(0)
        )
      );

      test("goes from 1st to 2nd", ({expect}) =>
        onStateChange(
          get_state({...initial_state.menu, commands}),
          Model.Actions.MenuNextItem,
          state =>
          expect.int(state.menu.selectedItem).toBe(1)
        )
      );
    });

    describe("MenuNextItem", ({test, _}) => {
      test("sets negative as last", ({expect}) =>
        onStateChange(
          get_state({...initial_state.menu, commands}),
          Model.Actions.MenuPosition(-2),
          state =>
          expect.int(state.menu.selectedItem).toBe(3)
        )
      );

      test("sets greater than lenght as 1st", ({expect}) =>
        onStateChange(
          get_state({...initial_state.menu, commands, selectedItem: 2}),
          Model.Actions.MenuPosition(List.length(commands) + 5),
          state =>
          expect.int(state.menu.selectedItem).toBe(0)
        )
      );

      test("sets position", ({expect}) =>
        onStateChange(
          get_state({...initial_state.menu, commands}),
          Model.Actions.MenuPosition(2),
          state =>
          expect.int(state.menu.selectedItem).toBe(2)
        )
      );
    });
  });
