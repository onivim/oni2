open Revery;
open Oni_Core;
open Revery.UI;
open Revery.UI.Components;
open Types;

let component = React.component("commandPalette");

let paletteWidth = 400;

let containerStyles = (theme: Theme.t) =>
  Style.[
    backgroundColor(theme.colors.editorMenuBackground),
    color(theme.colors.editorMenuForeground),
    width(paletteWidth),
    height(300),
    boxShadow(
      ~xOffset=-15.,
      ~yOffset=5.,
      ~blurRadius=30.,
      ~spreadRadius=5.,
      ~color=Color.rgba(0., 0., 0., 0.2),
    ),
  ];

let paletteItemStyle = Style.[fontSize(14)];

let createElement = (~children as _, ~menu: UiMenu.t, ~theme: Theme.t, ()) =>
  component(hooks =>
    (
      hooks,
      menu.isOpen ?
        <View style={containerStyles(theme)}>
          /* <Input style=Style.[width(paletteWidth)] /> */

            <ScrollView style=Style.[height(350)]>
              ...{
                   List.mapi(
                     (index, cmd: UiMenu.command) =>
                       <MenuItem
                         icon=""
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
