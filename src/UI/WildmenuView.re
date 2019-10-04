open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
open Oni_Model;

let component = React.component("wildmenu");

let menuWidth = 400;
let menuHeight = 320;

let createElement =
    (
      ~children as _,
      ~wildmenu: Wildmenu.t,
      ~theme: Theme.t,
      ~configuration,
      (),
    ) =>
  component(hooks => {
    let items = Array.of_list(wildmenu.items);
    let element =
      wildmenu.show
        ? <OniBoxShadow theme configuration>
            <FlatList
              rowHeight=40
              height=menuHeight
              width=menuWidth
              count={Array.length(items)}
              selected=Some(wildmenu.selected)
              render={index => {
                let item = items[index];
                <MenuItem
                  theme
                  label=item
                  selected={index == wildmenu.selected}
                  style=Style.[fontSize(16)]
                />;
              }}
            />
          </OniBoxShadow>
        : React.listToElement([]);
    (hooks, element);
  });
