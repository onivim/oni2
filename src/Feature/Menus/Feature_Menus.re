open Oni_Core;

module Log = (val Log.withNamespace("Oni.Feature.Menus"));

// BUILT-IN MENUS

let explorerContext = Menu.Lookup.get("explorer/context");
let editorContext = Menu.Lookup.get("editor/context");
let editorTitle = Menu.Lookup.get("editor/title");
let editorTitleContext = Menu.Lookup.get("editor/title/context");
let debugCallstackContext = Menu.Lookup.get("debug/callstack/context");
let debugToolbar = Menu.Lookup.get("debug/toolbar");
let scmTitle = Menu.Lookup.get("scm/title");
let scmResourceGroupContext = Menu.Lookup.get("scm/resourceGroup/context");
let scmResourceStateContext = Menu.Lookup.get("scm/resourceState/context");
let scmChangeTitle = Menu.Lookup.get("scm/change/title");
let viewTitle = Menu.Lookup.get("view/title");
let viewItemContext = Menu.Lookup.get("view/item/context");
let touchBar = Menu.Lookup.get("touchBar");
let commentThreadTitle = Menu.Lookup.get("comments/commentThread/title");
let commentThreadContext = Menu.Lookup.get("comments/commentThread/context");
let commentTitle = Menu.Lookup.get("comments/comment/title");
let commentContext = Menu.Lookup.get("comments/comment/context");

let commandPalette = (contextKeys, commands, menus: Menu.Lookup.t) => {
  // The command palette should contain all commands that are enabled and have
  // not been explicitly hidden by an associated menu item

  // Get enabled commands and convert them to menu items
  let commandItems =
    commands
    |> Command.Lookup.toList
    |> List.to_seq
    |> Seq.filter_map(
         Command.(
           command =>
             switch (command.title) {
             | Some(title)
                 when
                   WhenExpr.evaluate(
                     command.isEnabledWhen,
                     WhenExpr.ContextKeys.getValue(contextKeys),
                   ) =>
               Some((
                 command.id,
                 Menu.{
                   label: title,
                   category: command.category,
                   icon: command.icon,
                   isEnabledWhen: WhenExpr.Value(True), // disabled filtered out before
                   isVisibleWhen: WhenExpr.Value(True),
                   group: None,
                   index: None,
                   command: command.id,
                 },
               ))

             | _ => None
             }
         ),
       )
    |> StringMap.of_seq;

  // Get "commandPalette" menu definition
  let menuItems =
    Menu.Lookup.get("commandPalette", menus)
    |> List.to_seq
    |> Seq.map(Menu.(item => (item.command, item)))
    |> StringMap.of_seq;

  // Merge the two sets, hiding any commands explicitly hidden by a menu item
  StringMap.merge(
    (_id, commandItem, menuItem) =>
      switch (commandItem, menuItem) {
      | (Some(_), Some(menuItem: Menu.item))
          when
            WhenExpr.evaluate(
              menuItem.isVisibleWhen,
              WhenExpr.ContextKeys.getValue(contextKeys),
            ) =>
        None

      | (Some(commandItem), Some(menuItem: Menu.item)) =>
        Some(
          Menu.{...commandItem, group: menuItem.group, index: menuItem.index},
        )

      | (Some(commandItem), None) => Some(commandItem)

      | (None, Some(_)) => None // Should have been filtered out already

      | (None, None) => failwith("unreachable")
      },
    commandItems,
    menuItems,
  )
  |> StringMap.to_seq
  |> Seq.map(snd)
  |> List.of_seq;
};
