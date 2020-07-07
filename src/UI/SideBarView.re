open Revery.UI;

open Oni_Model;
module Core = Oni_Core;

open Feature_SideBar;
open Oni_Components;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let sidebar = (~width, ~theme, ~transition) => [
    flexDirection(`Row),
    backgroundColor(Colors.SideBar.background.from(theme)),
    Style.width(width),
    transform(Transform.[TranslateX(transition)]),
  ];

  let contents = [flexDirection(`Column), flexGrow(1)];

  let resizer = [flexGrow(0), width(4), position(`Relative)];

  let title = (~theme) => [color(Colors.SideBar.foreground.from(theme))];

  let heading = theme => [
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(Colors.SideBar.background.from(theme)),
    height(Core.Constants.tabHeight),
  ];
};

let animation =
  Revery.UI.Animation.(
    animate(Revery.Time.milliseconds(150))
    |> ease(Easing.easeIn)
    |> tween(-50.0, 0.)
    |> delay(Revery.Time.milliseconds(0))
  );

let%component make = (~theme, ~state: State.t, ~dispatch, ()) => {
  let State.{sideBar, uiFont: font, _} = state;

  let%hook (transition, _animationState, _reset) =
    Hooks.animation(animation, ~active=true);

  let title =
    switch (sideBar |> selected) {
    | FileExplorer => "Explorer"
    | SCM => "Source Control"
    | Extensions => "Extensions"
    };

  let elem =
    switch (sideBar |> selected) {
    | FileExplorer =>
      <FileExplorerView model={state.fileExplorer} theme font />

    | SCM =>
      let onItemClick = (resource: Feature_SCM.Resource.t) =>
        dispatch(
          Actions.OpenFileByPath(
            Oni_Core.Uri.toFileSystemPath(resource.uri),
            None,
            None,
          ),
        );

      <Feature_SCM.Pane
        model={state.scm}
        workingDirectory={state.workspace.workingDirectory}
        onItemClick
        isFocused={FocusManager.current(state) == Focus.SCM}
        theme
        font
        dispatch={msg => dispatch(Actions.SCM(msg))}
      />;

    | Extensions =>
      let extensionDispatch = msg => dispatch(Actions.Extensions(msg));
      <Feature_Extensions.ListView
        model={state.extensions}
        theme
        font
        isFocused={FocusManager.current(state) == Focus.Extensions}
        dispatch=extensionDispatch
      />;
    };

  let width = Feature_SideBar.width(state.sideBar);

  <View style={Styles.sidebar(~width, ~theme, ~transition)}>
    <View style=Styles.contents>
      <View style={Styles.heading(theme)}>
        <Text
          text=title
          style={Styles.title(~theme)}
          fontFamily={font.family}
          fontWeight=Medium
          fontSize={font.size}
        />
      </View>
      elem
    </View>
    <View style=Styles.resizer>
      <ResizeHandle.Vertical
        onDrag={delta =>
          dispatch(
            Actions.SideBar(
              Feature_SideBar.ResizeInProgress(int_of_float(delta)),
            ),
          )
        }
        onDragComplete={() =>
          dispatch(Actions.SideBar(Feature_SideBar.ResizeCommitted))
        }
      />
    </View>
  </View>;
};
