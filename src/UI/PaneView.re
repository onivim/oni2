open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
module Model = Oni_Model;
module Pane = Model.Pane;
module State = Model.State;

module Option = Utility.Option;

module Styles = {
  let pane = (~theme: Theme.t) =>
    Style.[
      flexDirection(`Row),
      height(200),
      borderTop(~color=theme.sideBarBackground, ~width=1),
    ];
};

let make = (~theme, ~uiFont, ~editorFont, ~state: State.t, ()) => {
  ignore(uiFont);
  ignore(editorFont);

  state.pane
  |> Pane.getType
  |> Option.map(paneType => {
       let childPane =
         switch (paneType) {
         | Pane.Search =>
           <SearchPane theme uiFont editorFont state={state.searchPane} />
         | Pane.Diagnostics =>
           <Container color=Colors.yellow width=100 height=100 />
         };
       <View style={Styles.pane(~theme)}> childPane </View>;
     })
  |> Option.value(~default=React.empty);
};
