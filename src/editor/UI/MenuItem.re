open Revery.UI;
open Revery.Core;
open Oni_Core;

let component = React.component("MenuItem");

let menuItemFontSize = 20;

let textStyles = (~selected, ~theme: Theme.t) =>
  Style.[
    fontFamily("FiraCode-Regular.ttf"),
    fontSize(menuItemFontSize),
    color(theme.editorMenuForeground),
    backgroundColor(
      selected ? theme.editorMenuItemSelected : Colors.transparentWhite,
    ),
  ];

let containerStyles = Style.[paddingHorizontal(10), flexDirection(`Row)];

let iconStyles =
  Style.[
    fontFamily("FontAwesome5FreeRegular.otf"),
    fontSize(menuItemFontSize),
    marginRight(10),
  ];

let createElement =
    (
      ~children as _,
      ~style=[],
      ~icon={||},
      ~label,
      ~selected=false,
      ~theme,
      (),
    ) =>
  component((_slots: React.Hooks.empty) => {
    let labelStyles =
      Style.(merge(~source=textStyles(~selected, ~theme), ~target=style));
    <View style=containerStyles>
      <Text style=iconStyles text=icon />
      <Text style=labelStyles text=label />
    </View>;
  });
