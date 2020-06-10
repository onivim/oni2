open Revery.UI;

open Oni_Model;
module Core = Oni_Core;

open Oni_Model.SideBar;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let container = (~theme, ~transition) =>
    Style.[
      backgroundColor(Colors.SideBar.background.from(theme)),
      width(225),
      transform(Transform.[TranslateX(transition)]),
    ];

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

let%component make = (~theme, ~state: State.t, ()) => {
  let State.{sideBar, uiFont: font, _} = state;

  let%hook (transition, _animationState, _reset) =
    Hooks.animation(animation, ~active=true);

  let title =
    switch (sideBar.selected) {
    | FileExplorer => "Explorer"
    | SCM => "Source Control"
    | Extensions => "Extensions"
    };

  let elem =
    switch (sideBar.selected) {
    | FileExplorer =>
      <FileExplorerView model={state.fileExplorer} theme font />

    | SCM =>
      let onItemClick = (resource: Feature_SCM.Resource.t) =>
        GlobalContext.current().dispatch(
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
        dispatch={msg => GlobalContext.current().dispatch(Actions.SCM(msg))}
      />;

    | Extensions => <ExtensionListView model={state.extensions} theme font />
    };

  <View style={Styles.container(~theme, ~transition)}>
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
  </View>;
};
