/**
   Editor Splits

   This modules interprets the current state of the window
   manager and arranges its children accordingly
 */
open Revery;
open UI;
open Oni_Model;
open Oni_Core.Types.EditorSplits;

let component = React.component("EditorSplits");

let spacer = (layout: layout) =>
  Style.(
    switch (layout) {
    | Full
    | VerticalRight
    | VerticalLeft => [
        backgroundColor(Colors.black),
        width(4),
        top(0),
        bottom(0),
        flexGrow(1),
      ]
    | HorizontalLeft
    | HorizontalRight => [
        backgroundColor(Colors.black),
        height(4),
        left(0),
        right(0),
        flexGrow(1),
      ]
    }
  );

let verticalStyles = (w, h, layout) =>
  Style.(
    switch (layout) {
    | Full => [top(0), bottom(0), flexGrow(1)]
    | VerticalLeft => [top(0), bottom(0), width(w)]
    | VerticalRight => [top(0), bottom(0), width(w)]
    | HorizontalLeft => [top(0), bottom(0), height(h)]
    | HorizontalRight => [top(0), bottom(0), height(h)]
    }
  );

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks =>
    (
      hooks,
      <View style=Style.[flexGrow(1), flexDirection(`Row)]>
        ...{
             WindowManager.toList(state.windows.splits)
             |> List.map(({width, height, component, layout, _}: split) => {
                  let splitStyles = verticalStyles(width, height, layout);
                  <View style=splitStyles> {component()} </View>;
                  /* <View style={spacer(layout)} /> */
                })
           }
      </View>,
    )
  );
