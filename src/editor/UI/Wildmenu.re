open Revery.UI;
open Revery.Core;
open Oni_Core;

let component = React.component("wildmenu");

let menuFontSize = 20;
let menuFontColor = Colors.white;

let textStyles = (~selected) =>
  Style.[
    fontFamily("FiraCode-Regular.ttf"),
    marginLeft(10),
    fontSize(menuFontSize),
    color(menuFontColor),
    backgroundColor(selected ? Colors.green : Colors.transparentWhite),
  ];

let containerStyles = (theme: Theme.t) =>
  Style.[
    width(400),
    height(500),
    backgroundColor(theme.editorLineNumberBackground),
    paddingVertical(20),
    overflow(LayoutTypes.Hidden),
  ];

let createElement =
    (~children as _, ~wildmenu: Types.Wildmenu.t, ~theme: Theme.t, ()) => {
  component((_slots: React.Hooks.empty) =>
    List.length(wildmenu.items) < 1
      ? React.listToElement([])
      : <View
          style=Style.[
            position(`Absolute),
            top(0),
            right(0),
            left(0),
            bottom(0),
            alignItems(`Center),
          ]>
          <View style={containerStyles(theme)}>
            ...{List.mapi(
              (index, item) => {
                let selected = index == wildmenu.selected;
                <Text style={textStyles(~selected)} text=item />;
              },
              wildmenu.items,
            )}
          </View>
        </View>
  );
};
