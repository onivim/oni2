open Revery;
open Oni_Core;
open Revery.UI;
open Revery.UI.Components;
open Types;

let component = React.component("Menu");

let menuWidth = 400;
let menuHeight = 300;

let containerStyles = (theme: Theme.t) =>
  Style.[
    backgroundColor(theme.colors.editorMenuBackground),
    color(theme.colors.editorMenuForeground),
    width(menuWidth),
    height(menuHeight),
    boxShadow(
      ~xOffset=-15.,
      ~yOffset=5.,
      ~blurRadius=30.,
      ~spreadRadius=5.,
      ~color=Color.rgba(0., 0., 0., 0.2),
    ),
  ];

let paletteItemStyle = Style.[fontSize(14)];

let getIcon = icon =>
  switch (icon) {
  | Some(i) => i
  | None => ""
  };

let createElement = (~children as _, ~menu: UiMenu.t, ~theme: Theme.t, ()) =>
  component(hooks =>
    (
      hooks,
      menu.isOpen ?
        <View style={containerStyles(theme)}>
          /* <Input style=Style.[width(paletteWidth)] /> */

            <ScrollView style=Style.[height(menuHeight)]>
              ...{
                   List.mapi(
                     (index, cmd: UiMenu.command) =>
                       <MenuItem
                         icon={getIcon(cmd.icon)}
                         style=paletteItemStyle
                         label={cmd.name}
                         selected={index == menu.selectedItem}
                         theme
                       />,
                     menu.commands,
                   )
                 }
            </ScrollView>
          </View> :
        React.listToElement([]),
    )
  );
