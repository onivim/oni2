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

let verticalStyles = (w, h) =>
  Style.[top(0), bottom(0), width(w), height(h)];

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks =>
    (
      hooks,
      <View style=Style.[flexGrow(1)]>
        ...{
             WindowManager.toList(state.windows.splits)
             |> List.map((split: split) =>
                  <View style={verticalStyles(split.width, split.height)}>
                    {split.component()}
                  </View>
                )
           }
      </View>,
    )
  );
