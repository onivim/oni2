open Revery.UI;
open Revery.UI.Components;
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
    height(300),
    backgroundColor(theme.editorLineNumberBackground),
    paddingVertical(20),
    overflow(LayoutTypes.Hidden),
  ];

let createElement =
    (~children as _, ~wildmenu: Types.Wildmenu.t, ~theme: Theme.t, ()) => {
  component((_slots: React.Hooks.empty) =>
    wildmenu.show
      ? <ScrollView style={containerStyles(theme)}>
          ...{List.mapi(
            (index, item) => {
              let selected = index == wildmenu.selected;
              <Text style={textStyles(~selected)} text=item />;
            },
            wildmenu.items,
          )}
        </ScrollView>
      : React.listToElement([])
  );
};
