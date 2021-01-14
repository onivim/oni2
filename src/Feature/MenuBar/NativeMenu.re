open Oni_Core;

module Internal = {
  let keyBindingsToKeyEquivalent:
    list(list(EditorInput.KeyPress.t)) =>
    option(Revery.Native.Menu.KeyEquivalent.t) =
    candidates => {
      candidates
      // Filter to single key press
      |> List.filter_map(
           fun
           | [hd] => Some(hd)
           | _ => None,
         )
      |> List.filter_map(EditorInput.KeyPress.toPhysicalKey)
      |> List.filter(({modifiers, _}: EditorInput.PhysicalKey.t) => {
           modifiers.super
         })
      |> (
        l =>
          List.nth_opt(l, 0)
          |> Option.map(({modifiers, key}: EditorInput.PhysicalKey.t) => {
               open Revery.Native.Menu;
               let key =
                 KeyEquivalent.ofString(EditorInput.Key.toString(key));

               let key =
                 if (modifiers.shift) {
                   KeyEquivalent.enableShift(key);
                 } else {
                   key;
                 };

               let key =
                 if (modifiers.alt) {
                   KeyEquivalent.enableAlt(key);
                 } else {
                   key;
                 };

               key;
             })
      );
    };
};

module Sub = {
  type menuParams = {
    builtMenu: MenuBar.builtMenu,
    getKeyEquivalent: string => option(Revery.Native.Menu.KeyEquivalent.t),
  };

  module OSXMenuSub =
    Isolinear.Sub.Make({
      type nonrec msg = string;
      type nonrec params = menuParams;
      type state = unit;

      let name = "Feature_Menu.nativeOSX";
      let id = (_: params) => "";

      let init = (~params, ~dispatch) => {
        module NativeMenu = Revery.Native.Menu;
        let menuBar = NativeMenu.getMenuBarHandle();

        let topItems = MenuBar.top(params.builtMenu |> MenuBar.schema);

        let rec buildItem =
                (parent: NativeMenu.t, items: list(MenuBar.Item.t)) => {
          items
          |> List.iter(item => {
               let title = MenuBar.Item.title(item);
               if (MenuBar.Item.isSubmenu(item)) {
                 let nativeMenu = NativeMenu.create(title);
                 NativeMenu.addSubmenu(~parent, ~child=nativeMenu);
                 buildGroup(nativeMenu, MenuBar.Item.submenu(item));
               } else {
                 ();
                 let command = MenuBar.Item.command(item);
                 let keyEquivalent =
                   params.getKeyEquivalent(command)
                   |> Option.value(
                        ~default=
                          Revery.Native.Menu.KeyEquivalent.ofString(""),
                      );

                 let nativeMenuItem =
                   Revery.Native.Menu.Item.create(
                     ~title,
                     ~onClick=() => {dispatch(command)},
                     ~keyEquivalent,
                     (),
                   );
                 Revery.Native.Menu.addItem(parent, nativeMenuItem);
               };
             });
        }
        and buildApplicationItems =
            (parent: NativeMenu.t, items: list(MenuBar.Item.t)) => {
          items
          |> List.iter(item => {
               let title = MenuBar.Item.title(item);
               if (MenuBar.Item.isSubmenu(item)) {
                 let nativeMenu = NativeMenu.create(title);
                 NativeMenu.insertSubmenuAt(
                   ~idx=1,
                   ~parent,
                   ~child=nativeMenu,
                 );
                 buildGroup(nativeMenu, MenuBar.Item.submenu(item));
               } else {
                 ();
                 let command = MenuBar.Item.command(item);
                 let keyEquivalent =
                   params.getKeyEquivalent(command)
                   |> Option.value(
                        ~default=
                          Revery.Native.Menu.KeyEquivalent.ofString(""),
                      );

                 let nativeMenuItem =
                   Revery.Native.Menu.Item.create(
                     ~title,
                     ~onClick=() => {dispatch(command)},
                     ~keyEquivalent,
                     (),
                   );
                 Revery.Native.Menu.insertItemAt(parent, nativeMenuItem, 1);
               };
             });
        }
        and buildGroup =
            (parent: NativeMenu.t, groups: list(MenuBar.Group.t)) => {
          let len = List.length(groups);
          groups
          |> List.iteri((idx, group) => {
               let isLast = idx == len - 1;
               let items = MenuBar.Group.items(group);
               buildItem(parent, items);

               if (!isLast) {
                 let separator = Revery.Native.Menu.Item.createSeparator();
                 Revery.Native.Menu.addItem(parent, separator);
               };
             });
        };
        topItems
        |> List.rev
        |> List.iter(item => {
             let title = MenuBar.Menu.title(item);

             if (MenuBar.Menu.uniqueId(item) == "help") {
               // Always add 'Help' last...
               let nativeMenu = NativeMenu.create(title);
               NativeMenu.addSubmenu(~parent=menuBar, ~child=nativeMenu);
               buildGroup(
                 nativeMenu,
                 MenuBar.Menu.contents(item, params.builtMenu),
               );
             } else if (MenuBar.Menu.uniqueId(item) == "application") {
               // Merge application items to existing bar
               let groups = MenuBar.Menu.contents(item, params.builtMenu);
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
                         let items = MenuBar.Group.items(group);
                         buildApplicationItems(appMenu, items);
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
                 nativeMenu,
                 MenuBar.Menu.contents(item, params.builtMenu),
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
      (~config, ~context, ~input, ~builtMenu: MenuBar.builtMenu, ~toMsg) =>
    if (Revery.Environment.isMac) {
      let getKeyEquivalent = command => {
        let bindings =
          Feature_Input.commandToAvailableBindings(
            ~config,
            ~context,
            ~command,
            input,
          );
        Internal.keyBindingsToKeyEquivalent(bindings);
      };
      OSXMenuSub.create({getKeyEquivalent, builtMenu})
      |> Isolinear.Sub.map(toMsg);
    } else {
      Isolinear.Sub.none;
    };
};

let sub = Sub.menu;
