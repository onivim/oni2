/*
 * DebugInputView.re
 *
 * UI to show internal input state
 */

open Revery.UI;

module Model = Oni_Model;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let row = [flexDirection(`Row)];

  let column = [flexDirection(`Column)];
};

let make = (~state: Model.State.t, ()) => {
  let contextKeys =
    Model.ContextKeys.all(state)
    |> WhenExpr.ContextKeys.values
    |> List.map(((keyName, value)) => {
         Printf.sprintf("%s: %s", keyName, WhenExpr.Value.asString(value))
       })
    |> List.sort(String.compare)
    |> List.map(text => <Text text />)
    |> React.listToElement;

  <View style=Styles.row>
    <View style=Styles.column>
      <Text text="Context Keys:" />
      contextKeys
    </View>
  </View>;
};
