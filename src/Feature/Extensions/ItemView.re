open Oni_Core;
open Revery.UI;

open Revery.UI.Components;

module Colors = Feature_Theme.Colors;

module Constants = {
  let itemHeight = 72;
  let imageSize = 50;
  let imageContainerSize = 64;
};

module Styles = {
  open Style;
  let container = (~width) => [
    Style.width(width),
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    height(Constants.itemHeight),
    flexGrow(0),
  ];
  let text = (~width, ~theme) => [
    color(Colors.SideBar.foreground.from(theme)),
    marginVertical(2),
    textOverflow(`Ellipsis),
    Style.width(width),
  ];

  let imageContainer =
    Style.[
      width(Constants.imageContainerSize),
      height(Constants.itemHeight),
      flexGrow(0),
      justifyContent(`Center),
      alignItems(`Center),
      flexDirection(`Column),
    ];
};

let make =
    (
      ~width,
      ~iconPath,
      ~theme,
      ~displayName,
      ~author,
      ~version,
      ~font: UiFont.t,
      (),
    ) => {
  let icon =
    switch (iconPath) {
    | None =>
      <Container
        color=Revery.Colors.darkGray
        width=Constants.imageSize
        height=Constants.imageSize
      />
    | Some(iconPath) =>
      <Image
        src={`File(iconPath)}
        width=Constants.imageSize
        height=Constants.imageSize
      />
    };

  let descriptionWidth = width - Constants.imageContainerSize;
  let defaultWidth = 100;

  <View style={Styles.container(~width)}>
    <View style=Styles.imageContainer> icon </View>
    <View>
      <Text
        style={Styles.text(~width=descriptionWidth, ~theme)}
        fontFamily={font.family}
        fontSize={font.size}
        text=displayName
      />
      <View
        style=Style.[
          flexDirection(`Row),
          justifyContent(`SpaceBetween),
          width(descriptionWidth),
        ]>
        <Text
          style={Styles.text(~width=defaultWidth, ~theme)}
          fontFamily={font.family}
          fontSize={font.size}
          text=author
        />
        <Text
          style={Styles.text(~width=defaultWidth, ~theme)}
          fontFamily={font.family}
          fontSize={font.size}
          text=version
        />
      </View>
    </View>
  </View>;
};
