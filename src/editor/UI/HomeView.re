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

/* let buttonStyles = (theme: Theme.t) => */
/*   Style.[ */
/*     color(theme.colors.foreground), */
/*     backgroundColor(theme.colors.background), */
/*   ]; */

let startEditor = () => GlobalContext.current().dispatch(ShowEditor);

let createElement = (~children as _, ~theme: Theme.t, ~state: State.t, ()) =>
  component(hooks => {
    let (animatedOpacity, hooks) =
      Hooks.animation(
        Animated.floatValue(0.),
        {
          toValue: 1.0,
          duration: Seconds(0.5),
          delay: Seconds(2.),
          repeat: false,
          easing: Animated.linear,
        },
        hooks,
      );
    let oniFontFamily = state.uiFont.fontFile;
    (
      hooks,
      <View style={containerStyles(theme)}>
        <View
          style=Style.[
            opacity(animatedOpacity),
            alignItems(`Center),
            justifyContent(`Center),
          ]>
          <Text
            text="Welcome to Oni2"
            style=Style.[fontFamily(oniFontFamily), fontSize(20)]
          />
          <Image
            src="logo.png"
            style=Style.[width(30), height(30), marginBottom(20)]
          />
          <Button
            title="Open Editor"
            fontFamily=oniFontFamily
            color={theme.colors.background}
            fontSize=20
            width=120
            height=40
            onClick=startEditor
          />
        </View>
      </View>,
    );
  });
