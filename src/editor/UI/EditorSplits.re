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
   1.) convert this to use new direction, simplifies this greatly
 */
let getSplitStyle = split =>
  Style.(
    switch (split) {
    | {direction: Vertical, width: Some(w), _} => [
        top(0),
        bottom(0),
        width(w),
      ]
    | {direction: Vertical, width: None, _} => [
        top(0),
        bottom(0),
        flexGrow(1),
      ]
    | {direction: Horizontal, height: None, _} => [
        left(0),
        right(0),
        flexGrow(1),
      ]
    | {direction: Horizontal, height: Some(h), _} => [
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
        (allSplits, split, direction) => [
          <View style={getSplitStyle(split)}> {split.component()} </View>,
          <WindowHandle direction theme={state.theme} />,
          ...allSplits,
        ],
        [],
        state.windows.splits,
        Vertical,
      );
    (hooks, <View style=splitContainer> ...splits </View>);
  });
