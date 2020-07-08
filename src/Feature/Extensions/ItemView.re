open Oni_Core;
open Revery;
open Revery.UI;

open Revery.UI.Components;

module Colors = Feature_Theme.Colors;

module Constants = {
  let itemHeight = 72;
};

module Styles = {
  open Style;
  let text = (~theme) => [
    color(Colors.SideBar.foreground.from(theme)),
    marginLeft(10),
    marginVertical(2),
    textWrap(TextWrapping.NoWrap),
    textOverflow(`Ellipsis),
  ];
  let input = [flexGrow(1), margin(12)];
};

let make =
    (~iconPath, ~theme, ~displayName, ~author, ~version, ~font: UiFont.t, ()) => {
  let icon =
    switch (iconPath) {
    | None => <Container color=Revery.Colors.darkGray width=32 height=32 />
    | Some(iconPath) => <Image src={`File(iconPath)} width=32 height=32 />
    };

  <View
    style=Style.[
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
      flexGrow(1),
      height(Constants.itemHeight),
    ]>
    icon
    <View style=Style.[flexDirection(`Column), flexGrow(1)]>
      <Text
        style={Styles.text(~theme)}
        fontFamily={font.family}
        fontSize={font.size}
        text=displayName
      />
      <View style=Style.[flexDirection(`Row), flexGrow(1)]>
        <View
          style=Style.[
            flexDirection(`Column),
            flexGrow(1),
            overflow(`Hidden),
          ]>
          <Text
            style={Styles.text(~theme)}
            fontFamily={font.family}
            fontSize={font.size}
            text=author
          />
        </View>
        <View
          style=Style.[
            flexDirection(`Column),
            flexGrow(1),
            overflow(`Hidden),
          ]>
          <Text
            style={Styles.text(~theme)}
            fontFamily={font.family}
            fontSize={font.size}
            text=version
          />
        </View>
      </View>
    </View>
  </View>;
};
