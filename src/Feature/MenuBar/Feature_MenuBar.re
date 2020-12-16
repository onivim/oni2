open Oni_Core;
open MenuBar;

[@deriving show]
type msg = 
| MouseOver({ uniqueId: string })
| MouseOut({ uniqueId: string })
| MouseClicked({ uniqueId: string });

type model = {
  menuSchema: Schema.t,
  activePath: option(string),
};

let update = (msg, model) => {
  switch (msg) {
  | MouseOver(_) => model
  
  | MouseOut(_) => model

  | MouseClicked({uniqueId}) => 
    ({
      ...model,
      activePath: Some(uniqueId)
    })
  }
}

let isActive = (uniqueId, model) => {
  switch (model.activePath) {
  | None => false
  | Some(activePath) => Utility.StringEx.contains(uniqueId, activePath)
  }
}

let initial = schema => {
  let global = Global.[application, file, edit, view] |> Schema.menus;

  {activePath: None, menuSchema: Schema.union(global, schema)};
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

  let topMenu = (~model, ~theme, ~dispatch, ~uniqueId, ~font: UiFont.t, ~title, ~color, ()) => {

    let contextMenu = Component_ContextMenu.(make(
      [
        {label: "test1", data: "test1"},
        {label: "test2", data: "test2"},
        {label: "test3", data: "test3"}
      ]
    ));
    let elem = isActive(uniqueId, model) ?
    <Component_ContextMenu.View
      model=contextMenu
      orientation=(`Bottom, `Left)
      dispatch={(_) => ()}
      theme
      font /> : React.empty;
    <View style=Style.[paddingHorizontal(6)] 
      onMouseDown={(_) => dispatch(MouseClicked({uniqueId: uniqueId}))}
    >
      <Text
        style={Styles.text(color)}
        text=title
        fontFamily={font.family}
        fontSize={font.size}
      />
      elem
    </View>;
  };

  let make =
      (
        ~isWindowFocused: bool,
        ~theme,
        ~font: UiFont.t,
        ~config as _,
        ~contextKeys,
        ~commands,
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

    let builtMenu = MenuBar.build(~contextKeys, ~commands, model.menuSchema);

    let topLevelMenuItems = MenuBar.top(builtMenu);

    let menuItems =
      topLevelMenuItems
      |> List.map(menu => {
           let title = MenuBar.Menu.title(menu);
           let uniqueId = MenuBar.Menu.uniqueId(menu);
           <topMenu model theme uniqueId dispatch font title color=fgColor />;
         })
      |> React.listToElement;

    <View style={Styles.container(bgColor)}> menuItems </View>;
  };
};
