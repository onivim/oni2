open Revery.UI;
open Revery.UI.Components;
open Revery.Core;
open Oni_Core;

let component = React.component("wildmenu");

let menuFontSize = 20;

let containerStyles = (theme: Theme.t) =>
  Style.[
    width(400),
    height(300),
    backgroundColor(theme.editorMenuBackground),
    paddingVertical(20),
    overflow(LayoutTypes.Hidden),
    boxShadow(
      ~xOffset=-15.,
      ~yOffset=5.,
      ~blurRadius=30.,
      ~spreadRadius=5.,
      ~color=Color.rgba(0., 0., 0., 0.2),
    ),
  ];

let createElement =
    (~children as _, ~wildmenu: Types.Wildmenu.t, ~theme: Theme.t, ()) => {
  component((_slots: React.Hooks.empty) =>
    wildmenu.show
      ? <ScrollView style={containerStyles(theme)}>
          ...{List.mapi(
            (index, item) => {
              let selected = index == wildmenu.selected;
              <MenuItem selected theme label=item />;
            },
            wildmenu.items,
          )}
        </ScrollView>
      : React.listToElement([])
  );
};
