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
    backgroundColor(theme.editorMenuBackground),
    paddingVertical(20),
    overflow(`Hidden),
  ];

let createElement =
    (~children as _, ~wildmenu: Wildmenu.t, ~theme: Theme.t, ()) =>
  component(hooks => {
    let element =
      wildmenu.show
        ? <OniBoxShadow>
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
          </OniBoxShadow>
        : React.listToElement([]);
    (hooks, element);
  });
