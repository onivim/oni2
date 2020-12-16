/*
 * DebugInputView.re
 *
 * UI to show internal input state
 */

open Revery.UI;
open Revery.UI.Components;

module Model = Oni_Model;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let row = [flexDirection(`Row), flexGrow(1)];

  let column = [flexDirection(`Column), flexGrow(1)];

  let section = [padding(16)];
};

let make = (~state: Model.State.t, ()) => {
  let context = Model.ContextKeys.all(state);
  let contextKeys =
    Model.ContextKeys.all(state)
    |> WhenExpr.ContextKeys.values
    |> List.map(((keyName, value)) => {
         Printf.sprintf("%s: %s", keyName, WhenExpr.Value.asString(value))
       })
    |> List.sort(String.compare)
    |> List.map(text => <Text text />)
    |> React.listToElement;

  let config = Model.Selectors.configResolver(state);

  let availableBindings =
    Feature_Input.candidates(~config, ~context, state.input);

  let bindingElems =
    availableBindings
    |> List.map(((matcher, command)) => {
         let cmd =
           switch (command) {
           | Feature_Input.VimExCommand(ex) => ex
           | Feature_Input.NamedCommand(cmd) => cmd
           };
         <View style=Styles.row>
           <View style=Style.[marginHorizontal(8)]>
             <Feature_Input.View.Matcher matcher font={state.uiFont} />
           </View>
           <Text text=cmd />
         </View>;
       })
    |> React.listToElement;

  let consumedKeys =
    Feature_Input.consumedKeys(state.input)
    |> List.map(Feature_Input.keyPressToString)
    |> String.concat(", ");

  <ScrollView style=Styles.row>
    <View style=Styles.section>
      <Text
        text={
          "Focus: "
          ++ (Model.FocusManager.current(state) |> Model.Focus.show_focusable)
        }
      />
      <Text text="Context Keys:" />
      contextKeys
    </View>
    <View style=Styles.section>
      <Text text="Consumed keys:" />
      <Text text=consumedKeys />
    </View>
    <View style=Styles.section>
      <Text text="Available bindings:" />
      bindingElems
    </View>
  </ScrollView>;
};
