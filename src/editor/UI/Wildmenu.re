open Revery.UI;
open Revery.Core;
open Oni_Core;

let component = React.component("wildmenu");

let menuFontSize = 20;
let menuFontColor = Colors.white;

let textStyles =
  Style.[
    fontFamily("FiraCode-Regular.ttf"),
    marginLeft(10),
    fontSize(menuFontSize),
    color(menuFontColor),
  ];

let createElement =
    (~children as _, ~wildmenu: Types.Wildmenu.t, ~theme as _: Theme.t, ()) => {
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
          ...{List.map(
            item => <Text style=textStyles text=item />,
            wildmenu.items,
          )}
        </View>
  );
};
