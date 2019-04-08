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

let lastItem = (l, index) => List.length(l) == index + 1;

let spacerColor = Color.rgba(0., 0., 0., 0.1);

let spacer = (layout: layout) =>
  Style.(
    switch (layout) {
    | Full
    | VerticalRight
    | VerticalLeft => [
        backgroundColor(spacerColor),
        width(1),
        top(0),
        bottom(0),
        flexGrow(0),
      ]
    | HorizontalTop
    | HorizontalBottom => [
        backgroundColor(spacerColor),
        height(1),
        left(0),
        right(0),
        flexGrow(0),
      ]
    }
  );

let verticalStyles = (w, h, layout) =>
  Style.(
    switch (layout) {
    | Full => [top(0), bottom(0), flexGrow(1)]
    | VerticalLeft => [top(0), bottom(0), width(w)]
    | VerticalRight => [top(0), bottom(0), width(w)]
    | HorizontalTop => [top(0), bottom(0), height(h)]
    | HorizontalBottom => [top(0), bottom(0), height(h)]
    }
  );

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks =>
    (
      hooks,
      <View style=Style.[flexGrow(1), flexDirection(`Row)]>
        ...{
             WindowManager.toList(state.windows.splits)
             |> (
               splits =>
                 List.mapi(
                   (index, {width, height, component, layout, _}: split) => {
                     let splitStyles = verticalStyles(width, height, layout);
                     [
                       <View style=splitStyles> {component()} </View>,
                       lastItem(splits, index) ?
                         React.empty : <View style={spacer(layout)} />,
                     ];
                   },
                   splits,
                 )
                 |> List.flatten
             )
           }
      </View>,
    )
  );
