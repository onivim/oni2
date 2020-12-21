open Oni_Core;
open MenuBar;

module Colors = Feature_Theme.Colors;

[@deriving show]
type msg =
  | MouseClicked({uniqueId: string})
  | ContextMenu(Component_ContextMenu.msg(string));

type session = {
  activePath: string,
  contextMenu: Component_ContextMenu.model(string),
};

type model = {
  menuSchema: Schema.t,
  activeSession: option(session),
};

type outmsg =
  | Nothing
  | ExecuteCommand({command: string});

let update = (~contextKeys, ~commands, msg, model) => {
  switch (msg) {
  | MouseClicked({uniqueId}) =>
    let builtMenu = MenuBar.build(~contextKeys, ~commands, model.menuSchema);
    let topLevelItems = MenuBar.top(model.menuSchema);
    let maybeMenu =
      topLevelItems
      |> List.filter((menu: Menu.t) => Menu.uniqueId(menu) == uniqueId)
      |> (l => List.nth_opt(l, 0));

    let session =
      maybeMenu
      |> Option.map(menu => {
           let contextMenu =
             Menu.contents(menu, builtMenu)
             |> List.map(group => {
                  let items =
                    MenuBar.Group.items(group)
                    |> List.map(item =>
                         Component_ContextMenu.{
                           label: Item.title(item),
                           data: Item.command(item),
                         }
                       );
                  //Component_ContextMenu.Group(items);
                  Component_ContextMenu.(
                    Submenu({
                      label: "test",
                      items: items |> List.map(i => Item(i)),
                    })
                  );
                })
             |> Component_ContextMenu.make;
           {activePath: uniqueId, contextMenu};
         });

    ({...model, activeSession: session}, Nothing);

  | ContextMenu(contextMenuMsg) =>
    let (activeSession', eff) =
      model.activeSession
      |> Option.map(session => {
           let (contextMenu', outmsg) =
             Component_ContextMenu.update(
               contextMenuMsg,
               session.contextMenu,
             );

           switch (outmsg) {
           | Component_ContextMenu.Nothing => (
               Some({...session, contextMenu: contextMenu'}),
               Nothing,
             )
           | Component_ContextMenu.Selected({data}) => (
               None,
               ExecuteCommand({command: data}),
             )
           | Component_ContextMenu.Cancelled => (None, Nothing)
           };
         })
      |> Option.value(~default=(model.activeSession, Nothing));

    ({...model, activeSession: activeSession'}, eff);
  };
};

let isActive = (uniqueId, model) => {
  switch (model.activeSession) {
  | None => false
  | Some({activePath, _}) => Utility.StringEx.contains(uniqueId, activePath)
  };
};

let initial = (~menus, ~groups) => {
  let contributedGroups = groups |> Schema.groups;
  let contributedMenus = Schema.menus(menus);
  let globalMenus = Global.menus |> Schema.menus;
  let globalGroups = Global.groups |> Schema.groups;

  {
    activeSession: None,
    menuSchema:
      Schema.ofList([
        globalMenus,
        globalGroups,
        contributedGroups,
        contributedMenus,
      ]),
  };
};

module Global = Global;
module View = {
  open Revery.UI;

  module Styles = {
    open Style;
    let container = bg => [
      height(30),
      backgroundColor(bg),
      flexDirection(`Row),
      justifyContent(`FlexStart),
      alignItems(`Center),
    ];

    let text = fg => [color(fg)];
  };

  module TopMenu = {
    let%component make =
                  (
                    ~model,
                    ~menu,
                    ~theme,
                    ~dispatch,
                    ~font: UiFont.t,
                    ~color,
                    ~backgroundColor,
                    (),
                  ) => {
      let%hook (isFocused, setIsFocused) = Hooks.state(false);

      let uniqueId = Menu.uniqueId(menu);
      let maybeElem =
        isActive(uniqueId, model)
          ? model.activeSession
            |> Option.map(({contextMenu, _}) => {
                 <Component_ContextMenu.View
                   model=contextMenu
                   orientation=(`Bottom, `Left)
                   dispatch={msg => dispatch(ContextMenu(msg))}
                   theme
                   font
                 />
               })
          : None;

      let elem = maybeElem |> Option.value(~default=React.empty);

      let bgColor =
        isFocused
          ? Colors.Menu.selectionBackground.from(theme) : backgroundColor;

      <View
        style=Style.[
          cursor(Revery.MouseCursors.pointer),
          paddingHorizontal(8),
          backgroundColor(bgColor),
        ]
        onMouseOut={_ => setIsFocused(_ => false)}
        onMouseOver={_ => setIsFocused(_ => true)}
        onMouseDown={_ => dispatch(MouseClicked({uniqueId: uniqueId}))}>
        <Text
          style={Styles.text(color)}
          text={Menu.title(menu)}
          fontFamily={font.family}
          fontSize={font.size}
        />
        elem
      </View>;
    };
  };

  let make =
      (
        ~isWindowFocused: bool,
        ~theme,
        ~font: UiFont.t,
        ~config as _,
        ~model,
        ~dispatch,
        (),
      ) => {
    let bgTheme =
      Feature_Theme.Colors.TitleBar.(
        isWindowFocused ? activeBackground : inactiveBackground
      );
    let fgTheme =
      Feature_Theme.Colors.TitleBar.(
        isWindowFocused ? activeForeground : inactiveForeground
      );
    let bgColor = bgTheme.from(theme);
    let fgColor = fgTheme.from(theme);

    let topLevelMenuItems = MenuBar.top(model.menuSchema);

    let menuItems =
      topLevelMenuItems
      |> List.map(menu => {
           <TopMenu
             model
             menu
             theme
             dispatch
             font
             color=fgColor
             backgroundColor=bgColor
           />
         })
      |> React.listToElement;

    <View style={Styles.container(bgColor)}> menuItems </View>;
  };
};
