open Revery.UI;
open Oni_Core;
open Oni_Model;
open Oni_Core_Utility;

let paneTabHeight = 25;

module Styles = {
  let pane = (~theme: Theme.t) =>
    Style.[
      flexDirection(`Column),
      height(225),
      borderTop(~color=theme.sideBarBackground, ~width=1),
      backgroundColor(theme.editorBackground),
    ];
};

let showSearch = () =>
  GlobalContext.current().dispatch(Actions.PaneTabClicked(Pane.Search));
let showProblems = () =>
  GlobalContext.current().dispatch(Actions.PaneTabClicked(Pane.Diagnostics));
let showNotifications = () =>
  GlobalContext.current().dispatch(
    Actions.PaneTabClicked(Pane.Notifications),
  );

let make = (~theme, ~uiFont, ~editorFont, ~state: State.t, ()) => {
  state.pane
  |> Pane.getType
  |> Option.map(paneType => {
       let childPane =
         switch (paneType) {
         | Pane.Search =>
           let onSelectResult = (file, location) =>
             GlobalContext.current().dispatch(
               Actions.OpenFileByPath(file, None, Some(location)),
             );
           let dispatch = msg =>
             GlobalContext.current().dispatch(Actions.Search(msg));

           <Feature_Search
             isFocused={FocusManager.current(state) == Focus.Search}
             theme
             uiFont
             editorFont
             model={state.searchPane}
             onSelectResult
             dispatch
           />;

         | Pane.Diagnostics => <DiagnosticsPane state />
         | Pane.Notifications => <NotificationsPane state />
         };
       [
         <WindowHandle theme direction=Horizontal />,
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
             <PaneTab
               uiFont
               theme
               title="Notifications"
               onClick=showNotifications
               active={paneType == Pane.Notifications}
             />
           </View>
           <View style=Style.[flexDirection(`Column), flexGrow(1)]>
             childPane
           </View>
         </View>,
       ]
       |> React.listToElement;
     })
  |> Option.value(~default=<View />);
};
