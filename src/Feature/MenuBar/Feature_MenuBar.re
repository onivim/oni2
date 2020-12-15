open Oni_Core;
open MenuBar;

[@deriving show]
type msg = unit;

type model = {menuSchema: Schema.t};

let update = (_msg, model) => model;

let initial = schema => {
  let global = Global.[application, file, edit, view] |> Schema.menus;

  {menuSchema: Schema.union(global, schema)};
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

  let topMenu = (~font: UiFont.t, ~title, ~color, ()) => {
    <View style=Style.[paddingHorizontal(6)]>
      <Text
        style={Styles.text(color)}
        text=title
        fontFamily={font.family}
        fontSize={font.size}
      />
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
           <topMenu font title color=fgColor />;
         })
      |> React.listToElement;

    <View style={Styles.container(bgColor)}> menuItems </View>;
  };
};
