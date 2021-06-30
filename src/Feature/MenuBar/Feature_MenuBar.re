open Oni_Core;
open ContextMenu;

module Colors = Feature_Theme.Colors;

module Configuration = {
  open Config.Schema;

  module Codec: {
    let menuBarVisibility:
      Config.Schema.codec(
        [
          // | `default
          | `visible
          // | `toggle
          | `hidden
          // | `compact
        ],
      );
  } = {
    let menuBarVisibility =
      custom(
        ~decode=
          Json.Decode.(
            string
            |> map(
                 fun
                 | "default" => `visible
                 | "visible" => `visible
                 | "toggle" => `visible
                 | "hidden" => `hidden
                 | "compact" => `visible
                 | _ => `visible,
               )
          ),
        ~encode=
          Json.Encode.(
            fun
            // | `default => string("default")
            | `visible => string("visible")
            // | `toggle => string("toggle")
            | `hidden => string("hidden")
          ),
        // | `compact => string("compact")
      );
  };

  let visibility =
    setting(
      "window.menuBarVisibility",
      Codec.menuBarVisibility,
      ~default=`visible,
    );
};

[@deriving show]
type msg =
  | MouseClicked({uniqueId: string})
  | MouseOver({uniqueId: string})
  | MouseOut({uniqueId: string})
  | ContextMenu(Component_ContextMenu.msg(string))
  | NativeMenu({command: string});

type session = {
  activePath: string,
  contextMenu: Component_ContextMenu.model(string),
};

type model = {
  menuSchema: Schema.t,
  activeSession: option(session),
  hoverId: option(string),
};

type outmsg =
  | Nothing
  | ExecuteCommand({command: string});

let show = (~contextKeys, ~commands, ~uniqueId, model) => {
  let builtMenu =
    ContextMenu.build(~contextKeys, ~commands, model.menuSchema);
  let topLevelItems = ContextMenu.top(model.menuSchema);
  let maybeMenu =
    topLevelItems
    |> List.filter((menu: Menu.t) => Menu.uniqueId(menu) == uniqueId)
    |> (l => List.nth_opt(l, 0));

  let session =
    maybeMenu
    |> Option.map(menu => {
         let contextMenu =
           Menu.contents(menu, builtMenu)
           |> List.map(Feature_ContextMenu.groupToContextMenu)
           |> Component_ContextMenu.make;
         {activePath: uniqueId, contextMenu};
       });
  {...model, activeSession: session};
};

let update = (~contextKeys, ~commands, msg, model) => {
  switch (msg) {
  | MouseClicked({uniqueId}) =>
    let model' =
      if (model.activeSession != None) {
        {...model, activeSession: None};
      } else {
        model |> show(~contextKeys, ~commands, ~uniqueId);
      };
    (model', Nothing);

  | MouseOver({uniqueId}) =>
    let model' =
      if (model.activeSession != None) {
        model |> show(~contextKeys, ~commands, ~uniqueId);
      } else {
        model;
      };
    ({...model', hoverId: Some(uniqueId)}, Nothing);

  | MouseOut({uniqueId}) =>
    let model' =
      if (model.hoverId == Some(uniqueId)) {
        {...model, hoverId: None};
      } else {
        model;
      };
    (model', Nothing);

  | NativeMenu({command}) => (model, ExecuteCommand({command: command}))

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
    hoverId: None,
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
    let make =
        (
          ~getShortcutKey,
          ~model,
          ~menu,
          ~theme,
          ~dispatch,
          ~font: UiFont.t,
          ~color,
          ~backgroundColor,
          (),
        ) => {
      let uniqueId = Menu.uniqueId(menu);
      let isFocused = Some(uniqueId) == model.hoverId;
      let maybeElem =
        isActive(uniqueId, model)
          ? model.activeSession
            |> Option.map(({contextMenu, _}) => {
                 let contextMenu' =
                   contextMenu
                   |> Feature_ContextMenu.augmentWithShortcutKeys(
                        ~getShortcutKey,
                      );

                 <Component_ContextMenu.View
                   model=contextMenu'
                   orientation=(`Top, `Left)
                   dispatch={msg => dispatch(ContextMenu(msg))}
                   theme
                   font
                 />;
               })
          : None;

      let elem = maybeElem |> Option.value(~default=React.empty);

      let bgColor =
        isFocused
          ? Colors.Menu.selectionBackground.from(theme) : backgroundColor;

      <View
        style=Style.[
          height(30),
          cursor(Revery.MouseCursors.arrow),
          paddingHorizontal(8),
          backgroundColor(bgColor),
          flexDirection(`Column),
          justifyContent(`Center),
          position(`Relative),
        ]
        onMouseOut={_ => dispatch(MouseOut({uniqueId: uniqueId}))}
        onMouseOver={_ => {dispatch(MouseOver({uniqueId: uniqueId}))}}
        onMouseDown={_ => dispatch(MouseClicked({uniqueId: uniqueId}))}>
        <View style=Style.[paddingTop(2)]>
          <Text
            style={Styles.text(color)}
            text={Menu.title(menu)}
            fontFamily={font.family}
            fontSize={font.size}
            fontWeight=Revery.Font.Weight.SemiBold
          />
        </View>
        <View
          style=Style.[
            position(`Absolute),
            bottom(0),
            left(0),
            width(1),
            height(1),
          ]>
          elem
        </View>
      </View>;
    };
  };

  let make =
      (
        ~isWindowFocused: bool,
        ~theme,
        ~font: UiFont.t,
        ~config,
        ~context,
        ~input,
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

    let topLevelMenuItems = ContextMenu.top(model.menuSchema);

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
                 style=Style.[color(fgColor), opacity(0.75)]
                 text=cmd
               />
             )
          |> Option.value(~default=Revery.UI.React.empty)
      );
    };

    let menuItems =
      topLevelMenuItems
      |> List.map(menu => {
           <TopMenu
             getShortcutKey
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

// SUBSCRIPTIOn

let sub = (~config, ~contextKeys, ~commands, ~input, model) => {
  let builtMenu =
    ContextMenu.build(~contextKeys, ~commands, model.menuSchema);
  NativeMenu.sub(
    ~config,
    ~context=contextKeys,
    ~input,
    ~toMsg=command => NativeMenu({command: command}),
    ~builtMenu,
  );
};

// CONTRIBUTIONS

module Contributions = {
  let configuration = Configuration.[visibility.spec];
};
