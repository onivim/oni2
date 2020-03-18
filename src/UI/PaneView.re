open Revery.UI;
open Oni_Core;
open Oni_Model;

module FontIcon = Oni_Components.FontIcon;
module FontAwesome = Oni_Components.FontAwesome;

module Constants = {
  let height = 225;
};

module Styles = {
  open Style;

  let pane = (~theme: Theme.t) => [
    flexDirection(`Column),
    height(Constants.height),
    borderTop(~color=theme.sideBarBackground, ~width=1),
    backgroundColor(theme.editorBackground),
  ];

  let header = [flexDirection(`Row), justifyContent(`SpaceBetween)];

  let tabs = [flexDirection(`Row)];

  let closeButton = [
    width(32),
    alignItems(`Center),
    justifyContent(`Center),
  ];

  let content = [flexDirection(`Column), flexGrow(1)];
};

let showSearch = () =>
  GlobalContext.current().dispatch(Actions.PaneTabClicked(Pane.Search));
let showProblems = () =>
  GlobalContext.current().dispatch(Actions.PaneTabClicked(Pane.Diagnostics));
let showNotifications = () =>
  GlobalContext.current().dispatch(
    Actions.PaneTabClicked(Pane.Notifications),
  );
let closePane = () =>
  GlobalContext.current().dispatch(Actions.PaneCloseButtonClicked);

let content = (~selected, ~theme, ~uiFont, ~editorFont, ~state, ()) =>
  switch (selected) {
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
  | Pane.Notifications =>
    let dispatch = msg =>
      GlobalContext.current().dispatch(Actions.Notification(msg));
    <Feature_Notification.View.Pane
      notifications={state.notifications}
      theme
      font=uiFont
      dispatch
    />;
  };

let closeButton = (~theme: Theme.t, ()) =>
  <Sneakable onClick=closePane style=Styles.closeButton>
    <FontIcon
      icon=FontAwesome.times
      color={theme.tabActiveForeground}
      fontSize=12.
    />
  </Sneakable>;

let make = (~theme, ~uiFont, ~editorFont, ~state: State.t, ()) =>
  if (!state.pane.isOpen) {
    <View />;
  } else {
    [
      <WindowHandle theme direction=Horizontal />,
      <View style={Styles.pane(~theme)}>
        <View style=Styles.header>
          <View style=Styles.tabs>
            <PaneTab
              uiFont
              theme
              title="Search"
              onClick=showSearch
              active={state.pane.selected == Pane.Search}
            />
            <PaneTab
              uiFont
              theme
              title="Problems"
              onClick=showProblems
              active={state.pane.selected == Pane.Diagnostics}
            />
            <PaneTab
              uiFont
              theme
              title="Notifications"
              onClick=showNotifications
              active={state.pane.selected == Pane.Notifications}
            />
          </View>
          <closeButton theme />
        </View>
        <View style=Styles.content>
          <content
            selected={state.pane.selected}
            theme
            uiFont
            editorFont
            state
          />
        </View>
      </View>,
    ]
    |> React.listToElement;
  };
