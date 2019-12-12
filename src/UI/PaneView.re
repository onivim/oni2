open Revery.UI;
open Oni_Core;
module Model = Oni_Model;
module Actions = Model.Actions;
module Pane = Model.Pane;
module State = Model.State;

module Option = Utility.Option;

let paneTabHeight = 25;

module Styles = {
  let pane = (~theme: Theme.t) =>
    Style.[
      flexDirection(`Column),
      height(225),
      borderTop(~color=theme.sideBarBackground, ~width=1),
    ];
};

let showSearch = _ =>
  GlobalContext.current().dispatch(Actions.PaneShow(Pane.Search));
let showProblems = _ =>
  GlobalContext.current().dispatch(Actions.PaneShow(Pane.Diagnostics));

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
         | Pane.Diagnostics => <DiagnosticsPane theme uiFont />
         };
       <View style={Styles.pane(~theme)}>
         <View style=Style.[flexDirection(`Row)]>
           <PaneTab
             uiFont
             theme
             title="Search"
             onClick=showSearch
             active={paneType == Pane.Search}
           />
           <PaneTab
             uiFont
             theme
             title="Problems"
             onClick=showProblems
             active={paneType == Pane.Diagnostics}
           />
         </View>
         <View style=Style.[flexDirection(`Column)]> childPane </View>
       </View>;
     })
  |> Option.value(~default=React.empty);
};
