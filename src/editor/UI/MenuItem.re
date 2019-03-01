open Revery;
open Revery.UI;
open Oni_Core;

let component = React.component("MenuItem");

let menuItemFontSize = 20;

let textStyles = (~theme: Theme.t) =>
  Style.[
    fontFamily("FiraCode-Regular.ttf"),
    fontSize(menuItemFontSize),
    color(theme.colors.editorMenuForeground),
  ];

let containerStyles = (~selected, ~theme: Theme.t) =>
  Style.[
    padding(10),
    flexDirection(`Row),
    backgroundColor(
      selected ? theme.colors.editorMenuItemSelected : Colors.transparentWhite,
    ),
  ];

let iconStyles =
  Style.[
    fontFamily("FontAwesome5FreeRegular.otf"),
    fontSize(menuItemFontSize),
    marginRight(10),
  ];

let createElement =
    (~children as _, ~style=[], ~icon={||}, ~label, ~selected, ~theme, ()) =>
  component(hooks => {
    let labelStyles =
      Style.(merge(~source=textStyles(~theme), ~target=style));
    (
      hooks,
      <View style={containerStyles(~theme, ~selected)}>
        <Text style=iconStyles text=icon />
        <Text style=labelStyles text=label />
      </View>,
    );
  });
