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

/**
   TODO:
   1.) width and height should be used as percentages of the
   window size not as direct values
   2.) This currently only handles halves not quarters of the screen size
 */
let getSplitStyle = ({layout, height: h, width: w, _}) =>
  Style.(
    switch (layout) {
    | Full => [top(0), bottom(0), flexGrow(1)]
    | VerticalLeft => [top(0), bottom(0), width(w)]
    | VerticalRight => [top(0), bottom(0), width(w)]
    | HorizontalTop => [left(0), right(0), height(h)]
    | HorizontalBottom => [left(0), right(0), height(h)]
    }
  );

let splitContainer = Style.[flexGrow(1), flexDirection(`Row)];

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks =>
    (
      hooks,
      <View style=splitContainer>
        ...{
             WindowManager.toList(state.windows.splits)
             |> (
               splits =>
                 List.mapi(
                   (i, split) => {
                     let style = getSplitStyle(split);
                     [
                       <View style> {split.component()} </View>,
                       <WindowHandle
                         splits
                         layout={split.layout}
                         windowNumber=i
                         theme={state.theme}
                       />,
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
