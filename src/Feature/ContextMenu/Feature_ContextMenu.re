open Oni_Core;
open ContextMenu;
module NativeMenu = Revery.Native.Menu;

module Native = Native;

type menu = {
  menuSchema: Schema.t,
  contextMenu: Component_ContextMenu.model(string),
  xPos: int,
  yPos: int,
};

type model = option(menu);

let initial = None;

let rec groupToContextMenu = (group: ContextMenu.Group.t) => {
  let items =
    ContextMenu.Group.items(group)
    |> List.map(item =>
         if (Item.isSubmenu(item)) {
           let submenuItems = Item.submenu(item);
           let groups = submenuItems |> List.map(groupToContextMenu);
           Component_ContextMenu.Submenu({
             label: Item.title(item),
             items: groups,
           });
         } else {
           Component_ContextMenu.Item({
             label: Item.title(item),
             data: Item.command(item),
             details: Revery.UI.React.empty,
           });
         }
       );

  Component_ContextMenu.Group(items);
};

let augmentWithShortcutKeys = (~getShortcutKey, contextMenu) =>
  contextMenu
  |> Component_ContextMenu.map(~f=item =>
       Component_ContextMenu.{
         ...item,
         label: item.label,
         details: getShortcutKey(item.data),
       }
     );

[@deriving show]
type msg =
  | MenuRequestedDisplayAt({
      [@opaque]
      menuSchema: Schema.t,
      xPos: int,
      yPos: int,
    })
  | ContextMenu(Component_ContextMenu.msg(string))
  | NativeMenuItemClicked(string);

type outmsg =
  | Nothing
  | ExecuteCommand({command: string})
  | Effect(Isolinear.Effect.t(msg));

module Effects = {
  let displayMenuAt = (~menuSchema: Schema.t, ~xPos: int, ~yPos: int) =>
    Isolinear.Effect.createWithDispatch(
      ~name="Feature_ContextMenu.displayMenuAt", dispatch =>
      dispatch(MenuRequestedDisplayAt({menuSchema, xPos, yPos}))
    );

  let displayNativeMenuAt =
      (~config, ~contextKeys, ~input, ~builtMenu, ~xPos, ~yPos, ~window) =>
    Isolinear.Effect.createWithDispatch(
      ~name="contextMenu.displayNativeMenuAt", dispatch => {
      let topLevelItems = ContextMenu.top(builtMenu |> ContextMenu.schema);

      Utility.OptionEx.iter2(
        (item, window) => {
          let title = ContextMenu.Menu.title(item);
          let nativeMenu = NativeMenu.create(title);
          Native.buildGroup(
            ~config,
            ~context=contextKeys,
            ~input,
            ~dispatch,
            nativeMenu,
            ContextMenu.Menu.contents(item, builtMenu),
          );
          Revery.Native.Menu.displayIn(
            ~x=xPos,
            ~y=yPos,
            nativeMenu,
            window |> Revery.Window.getSdlWindow,
          );
        },
        List.nth_opt(topLevelItems, 0),
        window,
      );
    });
};

let update = (~contextKeys, ~commands, ~config, ~input, ~window, msg, model) =>
  switch (msg) {
  | MenuRequestedDisplayAt({menuSchema, xPos, yPos}) =>
    let builtMenu = ContextMenu.build(~contextKeys, ~commands, menuSchema);

    if (Revery.Environment.isMac) {
      let eff =
        Effects.displayNativeMenuAt(
          ~config,
          ~contextKeys,
          ~input,
          ~builtMenu,
          ~xPos,
          ~yPos,
          ~window,
        )
        |> Isolinear.Effect.map(str => NativeMenuItemClicked(str));
      (None, Effect(eff));
    } else {
      let topLevelItems = ContextMenu.top(menuSchema);
      let maybeMenu = List.nth_opt(topLevelItems, 0);

      let menu =
        maybeMenu
        |> Option.map(menu => {
             let contextMenu =
               Menu.contents(menu, builtMenu)
               |> List.map(groupToContextMenu)
               |> Component_ContextMenu.make;
             {menuSchema, xPos, yPos, contextMenu};
           });
      (menu, Nothing);
    };
  | NativeMenuItemClicked(command) => (
      model,
      ExecuteCommand({command: command}),
    )
  | ContextMenu(contextMenuMsg) =>
    let (model', eff) =
      model
      |> Option.map(menu => {
           let (contextMenu', outmsg) =
             Component_ContextMenu.update(contextMenuMsg, menu.contextMenu);

           switch (outmsg) {
           | Component_ContextMenu.Nothing => (
               Some({...menu, contextMenu: contextMenu'}),
               Nothing,
             )
           | Component_ContextMenu.Selected({data}) => (
               None,
               ExecuteCommand({command: data}),
             )
           | Component_ContextMenu.Cancelled => (None, Nothing)
           };
         })
      |> Option.value(~default=(model, Nothing));

    (model', eff);
  };

module View = {
  open Revery.UI;

  module Styles = {
    open Style;
    let overlay = [
      position(`Absolute),
      top(0),
      left(0),
      bottom(0),
      right(0),
    ];

    let coords = (~x, ~y) => [position(`Absolute), left(x), top(y)];
  };
  let make =
      (
        ~contextMenu as model,
        ~config,
        ~context,
        ~input,
        ~theme,
        ~font: UiFont.t,
        ~dispatch,
        (),
      ) => {
    let getShortcutKey = command => {
      Feature_Input.commandToAvailableBindings(
        ~command,
        ~config,
        ~context,
        input,
      )
      |> (
        l =>
          List.nth_opt(l, 0)
          |> Option.map(keys =>
               keys
               |> List.map(Feature_Input.keyPressToString)
               |> String.concat(" ")
             )
          |> Utility.OptionEx.or_lazy(() =>
               if (Utility.StringEx.startsWith(~prefix=":", command)) {
                 Some(command);
               } else {
                 None;
               }
             )
          |> Option.map(cmd =>
               <Text
                 fontFamily={font.family}
                 fontSize=11.
                 style=Style.[
                   color(Feature_Theme.Colors.Menu.foreground.from(theme)),
                   opacity(0.75),
                 ]
                 text=cmd
               />
             )
          |> Option.value(~default=Revery.UI.React.empty)
      );
    };

    let elem =
      model
      |> Option.map(({contextMenu, xPos, yPos, _}) => {
           let contextMenu =
             contextMenu |> augmentWithShortcutKeys(~getShortcutKey);
           <View style={Styles.coords(~x=xPos + 1, ~y=yPos + 1)}>
             <Component_ContextMenu.View
               model=contextMenu
               orientation=(`Top, `Left)
               dispatch={msg => dispatch(ContextMenu(msg))}
               theme
               font
             />
           </View>;
         })
      |> Option.value(~default=React.empty);
    <View style=Styles.overlay> elem </View>;
  };
};
