open Revery.UI;
open Oni_Model;
open Oni_Components;

module FontIcon = Oni_Components.FontIcon;
module FontAwesome = Oni_Components.FontAwesome;
module Sneakable = Feature_Sneak.View.Sneakable;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let pane = (~theme, ~height) => [
    flexDirection(`Column),
    Style.height(height),
    borderTop(~color=Colors.Panel.border.from(theme), ~width=1),
    backgroundColor(Colors.Panel.background.from(theme)),
  ];

  let header = [flexDirection(`Row), justifyContent(`SpaceBetween)];

  let tabs = [flexDirection(`Row)];

  let closeButton = [
    width(32),
    alignItems(`Center),
    justifyContent(`Center),
  ];

  let resizer = [height(4), position(`Relative), flexGrow(0)];

  let content = [flexDirection(`Column), flexGrow(1)];
};

let showSearch = () =>
  GlobalContext.current().dispatch(
    Actions.PaneTabClicked(Feature_Pane.Search),
  );
let showProblems = () =>
  GlobalContext.current().dispatch(
    Actions.PaneTabClicked(Feature_Pane.Diagnostics),
  );
let showNotifications = () =>
  GlobalContext.current().dispatch(
    Actions.PaneTabClicked(Feature_Pane.Notifications),
  );
let closePane = () =>
  GlobalContext.current().dispatch(Actions.PaneCloseButtonClicked);

let content = (~selected, ~theme, ~uiFont, ~editorFont, ~state: State.t, ()) =>
  switch (selected) {
  | Feature_Pane.Search =>
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

  | Feature_Pane.Diagnostics =>
    <DiagnosticsPane diagnostics={state.diagnostics} theme uiFont editorFont />
  | Feature_Pane.Notifications =>
    let dispatch = msg =>
      GlobalContext.current().dispatch(Actions.Notification(msg));
    <Feature_Notification.View.List
      model={state.notifications}
      theme
      font=uiFont
      dispatch
    />;
  };

let closeButton = (~theme, ()) =>
  <Sneakable onClick=closePane style=Styles.closeButton>
    <FontIcon
      icon=FontAwesome.times
      color={Colors.Tab.activeForeground.from(theme)}
      fontSize=12.
    />
  </Sneakable>;

let make = (~theme, ~uiFont, ~editorFont, ~state: State.t, ()) => {
  let dispatch = GlobalContext.current().dispatch;
  if (!Feature_Pane.isOpen(state.pane)) {
    <View />;
  } else {
    let height = Feature_Pane.height(state.pane);
    <View style={Styles.pane(~theme, ~height)}>
      <View style=Styles.resizer>
        <ResizeHandle.Horizontal
          onDrag={delta =>
            dispatch(
              Actions.Pane(
                Feature_Pane.Msg.resizeHandleDragged(int_of_float(delta)),
              ),
            )
          }
          onDragComplete={() =>
            dispatch(Actions.Pane(Feature_Pane.Msg.resizeCommitted))
          }
        />
      </View>
      <View style=Styles.header>
        <View style=Styles.tabs>
          <PaneTab
            uiFont
            theme
            title="Search"
            onClick=showSearch
            isActive={Feature_Pane.isVisible(Feature_Pane.Search, state.pane)}
          />
          <PaneTab
            uiFont
            theme
            title="Problems"
            onClick=showProblems
            isActive={Feature_Pane.isVisible(
              Feature_Pane.Diagnostics,
              state.pane,
            )}
          />
          <PaneTab
            uiFont
            theme
            title="Notifications"
            onClick=showNotifications
            isActive={Feature_Pane.isVisible(
              Feature_Pane.Notifications,
              state.pane,
            )}
          />
        </View>
        <closeButton theme />
      </View>
      <View style=Styles.content>
        <content
          selected={Feature_Pane.selected(state.pane)}
          theme
          uiFont
          editorFont
          state
        />
      </View>
    </View>;
  };
};
