open Oni_Core;

module Sub = {
  type menuParams = {
    builtMenu: ContextMenu.builtMenu,
    config: Oni_Core.Config.resolver,
    context: WhenExpr.ContextKeys.t,
    input: Feature_Input.model,
  };

  module OSXMenuSub =
    Isolinear.Sub.Make({
      open Feature_ContextMenu.Native;

      type nonrec msg = string;
      type nonrec params = menuParams;
      type state = unit;

      let name = "Feature_Menu.nativeOSX";
      let id = (_: params) => "";

      let init = (~params, ~dispatch) => {
        module NativeMenu = Revery.Native.Menu;
        let menuBar = NativeMenu.getMenuBarHandle();

        let topItems =
          ContextMenu.top(params.builtMenu |> ContextMenu.schema);

        topItems
        |> List.rev
        |> List.iter(item => {
             let title = ContextMenu.Menu.title(item);

             if (ContextMenu.Menu.uniqueId(item) == "help") {
               // Always add 'Help' last...
               let nativeMenu = NativeMenu.create(title);
               NativeMenu.addSubmenu(~parent=menuBar, ~child=nativeMenu);
               buildGroup(
                 ~config=params.config,
                 ~context=params.context,
                 ~input=params.input,
                 ~dispatch,
                 nativeMenu,
                 ContextMenu.Menu.contents(item, params.builtMenu),
               );
             } else if (ContextMenu.Menu.uniqueId(item) == "application") {
               // Merge application items to existing bar
               let groups = ContextMenu.Menu.contents(item, params.builtMenu);
               let maybeAppMenu =
                 NativeMenu.nth(menuBar, 0)
                 |> Utility.OptionEx.flatMap(NativeMenu.Item.getSubmenu);

               maybeAppMenu
               |> Option.iter(appMenu => {
                    // First, remove existing 'preferences' item

                    let maybeExistingPreferences = NativeMenu.nth(appMenu, 2);
                    maybeExistingPreferences
                    |> Option.iter(pref =>
                         Revery.Native.Menu.removeItem(appMenu, pref)
                       );

                    groups
                    |> List.iter(group => {
                         let items = ContextMenu.Group.items(group);
                         buildApplicationItems(
                           ~config=params.config,
                           ~context=params.context,
                           ~input=params.input,
                           ~dispatch,
                           appMenu,
                           items,
                         );
                         let separator =
                           Revery.Native.Menu.Item.createSeparator();
                         Revery.Native.Menu.insertItemAt(
                           appMenu,
                           separator,
                           1,
                         );
                       });
                  });
             } else {
               // But other menu items - add before the existing 'Window' menu item
               let nativeMenu = NativeMenu.create(title);
               NativeMenu.insertSubmenuAt(
                 ~idx=1,
                 ~parent=menuBar,
                 ~child=nativeMenu,
               );
               buildGroup(
                 ~config=params.config,
                 ~context=params.context,
                 ~input=params.input,
                 ~dispatch,
                 nativeMenu,
                 ContextMenu.Menu.contents(item, params.builtMenu),
               );
             };
           });
        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });

  let menu =
      (~config, ~context, ~input, ~builtMenu: ContextMenu.builtMenu, ~toMsg) =>
    if (Revery.Environment.isMac) {
      OSXMenuSub.create({builtMenu, config, context, input})
      |> Isolinear.Sub.map(toMsg);
    } else {
      Isolinear.Sub.none;
    };
};

let sub = Sub.menu;
