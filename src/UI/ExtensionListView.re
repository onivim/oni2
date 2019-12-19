open Oni_Model;
open Revery;
open Revery.UI;
open Revery.UI.Components;

module Option = Oni_Core.Utility.Option;

module Styles = {
  let container = Style.[
    flexGrow(1),
  ];
}

let make = (~state: State.t, ()) => {

  let renderItem = i => {
    <Container color=Colors.yellow width=45 height=45 /> 
  };

  <View style=Styles.container>
    <FlatList
      rowHeight={50}
      count={100}
      render=renderItem
      focused=None
      />
  </View>
};
