open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
open Oni_Model;

let component = React.component("wildmenu");

let containerStyles = (theme: Theme.t) =>
  Style.[
    width(400),
    height(300),
    backgroundColor(theme.colors.editorMenuBackground),
    paddingVertical(20),
    overflow(`Hidden),
  ];

let createElement =
    (~children as _, ~wildmenu: Wildmenu.t, ~theme: Theme.t, ()) =>
  component(hooks => {
    let element =
      wildmenu.show
        ? <BoxShadow
            boxShadow={Style.BoxShadow.make(
              ~xOffset=-15.,
              ~yOffset=5.,
              ~blurRadius=30.,
              ~spreadRadius=5.,
              ~color=Color.rgba(0., 0., 0., 0.2),
              (),
            )}>
            <ScrollView style={containerStyles(theme)}>
              ...{List.mapi(
                (index, item) =>
                  <MenuItem
                    theme
                    label=item
                    selected={index == wildmenu.selected}
                    style=Style.[fontSize(16)]
                  />,
                wildmenu.items,
              )}
            </ScrollView>
          </BoxShadow>
        : React.listToElement([]);
    (hooks, element);
  });
