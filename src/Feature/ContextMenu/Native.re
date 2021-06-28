open Oni_Core;
module NativeMenu = Revery.Native.Menu;

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
               KeyEquivalent.ofString(EditorInput.Key.toString(key))
               |> (key => KeyEquivalent.enableShift(key, modifiers.shift))
               |> (key => KeyEquivalent.enableAlt(key, modifiers.alt))
               |> (key => KeyEquivalent.enableCtrl(key, modifiers.control));

             key;
           })
    );
  };

let getKeyEquivalent = (~config, ~context, ~input, command) =>
  switch (Revery.Environment.os) {
  | Mac(_) =>
    Feature_Input.commandToAvailableBindings(
      ~command,
      ~config,
      ~context,
      input,
    )
    |> keyBindingsToKeyEquivalent

  | _ => None
  };

let rec buildItem =
        (
          ~config,
          ~context,
          ~input,
          ~dispatch,
          parent: NativeMenu.t,
          items: list(ContextMenu.Item.t),
        ) => {
  items
  |> List.iter(item => {
       let title = ContextMenu.Item.title(item);
       if (ContextMenu.Item.isSubmenu(item)) {
         let nativeMenu = NativeMenu.create(title);
         NativeMenu.addSubmenu(~parent, ~child=nativeMenu);
         buildGroup(
           ~config,
           ~context,
           ~input,
           ~dispatch,
           nativeMenu,
           ContextMenu.Item.submenu(item),
         );
       } else {
         let command = ContextMenu.Item.command(item);
         let keyEquivalent =
           getKeyEquivalent(~config, ~context, ~input, command)
           |> Option.value(
                ~default=Revery.Native.Menu.KeyEquivalent.ofString(""),
              );

         let nativeMenuItem =
           Revery.Native.Menu.Item.create(
             ~title,
             ~onClick=
               (~fromKeyPress, ()) =>
                 if (!fromKeyPress) {
                   dispatch(command);
                 },
             ~keyEquivalent,
             (),
           );
         Revery.Native.Menu.addItem(parent, nativeMenuItem);
       };
     });
}
and buildGroup =
    (
      ~config,
      ~context,
      ~input,
      ~dispatch,
      parent: NativeMenu.t,
      groups: list(ContextMenu.Group.t),
    ) => {
  let len = List.length(groups);
  groups
  |> List.iteri((idx, group) => {
       let isLast = idx == len - 1;
       let items = ContextMenu.Group.items(group);
       buildItem(~config, ~context, ~input, ~dispatch, parent, items);

       if (!isLast) {
         let separator = Revery.Native.Menu.Item.createSeparator();
         Revery.Native.Menu.addItem(parent, separator);
       };
     });
};

let buildApplicationItems =
    (
      ~config,
      ~context,
      ~input,
      ~dispatch,
      parent: NativeMenu.t,
      items: list(ContextMenu.Item.t),
    ) => {
  items
  |> List.iter(item => {
       let title = ContextMenu.Item.title(item);
       if (ContextMenu.Item.isSubmenu(item)) {
         let nativeMenu = NativeMenu.create(title);
         NativeMenu.insertSubmenuAt(~idx=1, ~parent, ~child=nativeMenu);
         buildGroup(
           ~config,
           ~context,
           ~input,
           ~dispatch,
           nativeMenu,
           ContextMenu.Item.submenu(item),
         );
       } else {
         ();
         let command = ContextMenu.Item.command(item);
         let keyEquivalent =
           getKeyEquivalent(~config, ~context, ~input, command)
           |> Option.value(
                ~default=Revery.Native.Menu.KeyEquivalent.ofString(""),
              );

         let nativeMenuItem =
           Revery.Native.Menu.Item.create(
             ~title,
             ~onClick=
               (~fromKeyPress, ()) =>
                 if (!fromKeyPress) {
                   dispatch(command);
                 },
             ~keyEquivalent,
             (),
           );
         Revery.Native.Menu.insertItemAt(parent, nativeMenuItem, 1);
       };
     });
};
