/**
   Editor Splits

   This modules interprets the current state of the window
   manager and arranges its children accordingly
 */
open Revery;
open UI;
open Oni_Model;
open WindowManager;

let component = React.component("EditorSplits");

/**
   TODO:
   1.) This currently only handles halves not quarters of the screen size
 */
let getSplitStyle = split =>
  Style.(
    switch (split) {
    | {layout: VerticalRight, width: Some(w), _}
    | {layout: VerticalLeft, width: Some(w), _} => [
        top(0),
        bottom(0),
        width(w),
      ]
    | {layout: VerticalRight, width: None, _}
    | {layout: VerticalLeft, width: None, _} => [
        top(0),
        bottom(0),
        flexGrow(1),
      ]
    | {layout: HorizontalBottom, height: None, _}
    | {layout: HorizontalTop, height: None, _} => [
        left(0),
        right(0),
        flexGrow(1),
      ]
    | {layout: HorizontalTop, height: Some(h), _}
    | {layout: HorizontalBottom, height: Some(h), _} => [
        left(0),
        right(0),
        height(h),
        flexGrow(1),
      ]
    }
  );

let splitContainer = Style.[flexGrow(1), flexDirection(`Row)];

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks => {
    let splits =
      WindowManager.traverseSplitTree(
        state.windows.splits,
        (split, allSplits) => [
          <View style={getSplitStyle(split)}> {split.component()} </View>,
          <WindowHandle layout={split.layout} theme={state.theme} />,
          ...allSplits,
        ],
        [],
      );
    (hooks, <View style=splitContainer> ...splits </View>);
  });
