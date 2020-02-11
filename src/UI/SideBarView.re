open Revery.UI;

open Oni_Model;
module Core = Oni_Core;

open Oni_Model.SideBar;
module Styles = {
  open Style;

  let container = (~bg, ~transition) =>
    Style.[
      backgroundColor(bg),
      width(225),
      transform(Transform.[TranslateX(transition)]),
    ];

  let title = (~fg, ~bg, ~font: Core.UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFileSemiBold),
    backgroundColor(bg),
    color(fg),
  ];

  let heading = (theme: Core.Theme.t) => [
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(theme.sideBarBackground),
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

let%component make = (~state: State.t, ()) => {
  [@warning "-27"]
  let State.{theme, sideBar, uiFont: font, _} = state;

  let bg = theme.sideBarBackground;
  let fg = theme.sideBarForeground;

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
    | FileExplorer => <FileExplorerView state />
    | SCM =>
      let onItemClick = (resource: Oni_Core.SCMResource.t) =>
        GlobalContext.current().dispatch(
          Actions.OpenFileByPath(Oni_Core.Uri.toFileSystemPath(resource.uri), None, None),
        );

      let workingDirectory = Option.map(w => w.Workspace.workingDirectory, state.workspace);

      <Feature_SCM.Pane model=state.scm workingDirectory onItemClick theme font />

    | Extensions => <ExtensionListView state />
    };

  <View style={Styles.container(~bg, ~transition)}>
    <View style={Styles.heading(theme)}>
      <Text text=title style={Styles.title(~fg, ~bg, ~font)} />
    </View>
    elem
  </View>;
};
