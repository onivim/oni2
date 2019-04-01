open Revery.UI;
open Revery.UI.Components;
open Oni_Model;
open Oni_Core;

let component = React.component("Home");

let containerStyles = (theme: Theme.t) =>
  Style.[
    backgroundColor(theme.colors.background),
    color(theme.colors.foreground),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
    flexDirection(`Column),
    alignItems(`Center),
    justifyContent(`Center),
  ];

let buttonStyles = (theme: Theme.t) =>
  Style.[color(theme.colors.foreground)];

let startEditor = () => GlobalContext.current().dispatch(ShowHome);

let createElement = (~children as _, ~theme: Theme.t, ~state: State.t, ()) =>
  component(hooks => {
    let oniFontFamily = state.uiFont.fontFile;
    (
      hooks,
      <View style={containerStyles(theme)}>
        <Image src="logo.png" style=Style.[width(30), height(30)] />
        <Text
          text="Start Screen"
          style=Style.[fontFamily(oniFontFamily), fontSize(20)]
        />
        <Button
          title="Open Editor"
          fontFamily=oniFontFamily
          onClick=startEditor
        />
      </View>,
    );
  });
