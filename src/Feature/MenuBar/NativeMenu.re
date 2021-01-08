open Oni_Core;

module Sub = {
  type menuParams = {builtMenu: MenuBar.builtMenu};

  module OSXMenuSub =
    Isolinear.Sub.Make({
      type nonrec msg = string;
      type nonrec params = menuParams;
      type state = unit;

      let name = "Feature_Menu.nativeOSX";
      let id = (_: params) => "";

      let init = (~params as _, ~dispatch as _) => {
        open Revery.Native;
        let menuBar = Menu.getMenuBarHandle();

        let menu1 = Menu.create("Test 1");
        Menu.addSubmenu(~parent=menuBar, ~child=menu1);
        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });

  let menu = (~builtMenu: MenuBar.builtMenu, ~toMsg) =>
    if (Revery.Environment.isMac) {
      OSXMenuSub.create({builtMenu: builtMenu}) |> Isolinear.Sub.map(toMsg);
    } else {
      Isolinear.Sub.none;
    };
};

let sub = Sub.menu;
