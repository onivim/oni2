open Oni_Core;

type model = unit;

let initial = ();

module Global = Global;

module View = {
  open Revery;
  open Revery.UI;
  open Revery.UI.Components;

  module Styles = {
    open Style;
    let container = bg => [
      height(30),
      backgroundColor(bg),
      flexDirection(`Row),
      justifyContent(`FlexStart),
      alignItems(`Center),
    ];

    let text = fg => [color(fg)];
  };

  let make =
      (
        ~isWindowFocused: bool,
        ~theme,
        ~font: UiFont.t,
        ~config as _,
        ~model as _,
        (),
      ) => {
    let bgTheme =
      Feature_Theme.Colors.TitleBar.(
        isWindowFocused ? activeBackground : inactiveBackground
      );
    let fgTheme =
      Feature_Theme.Colors.TitleBar.(
        isWindowFocused ? activeForeground : inactiveForeground
      );
    let bgColor = bgTheme.from(theme);
    let fgColor = fgTheme.from(theme);
    <View style={Styles.container(bgColor)}>
      <Text
        style={Styles.text(fgColor)}
        text="Hello, world!"
        fontFamily={font.family}
        fontSize={font.size}
      />
    </View>;
  };
};
