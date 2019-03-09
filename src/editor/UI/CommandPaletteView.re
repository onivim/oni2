open Revery;
open Oni_Core;
open Revery.UI;
open Revery.UI.Components;

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

let createElement =
    (~children as _, ~commandPalette: CommandPalette.t, ~theme: Theme.t, ()) =>
  component(hooks =>
    (
      hooks,
      commandPalette.isOpen ?
        <ScrollView style={containerStyles(theme)}>
          <Input style=Style.[width(paletteWidth)] />
          <MenuItem label="CommandPalette" selected=true theme />
        </ScrollView> :
        React.listToElement([]),
    )
  );
