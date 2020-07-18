open Oni_Core;
open Revery.UI;

open Revery.UI.Components;

module Colors = Feature_Theme.Colors;
module Sneakable = Feature_Sneak.View.Sneakable;

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
  let titleText = (~width, ~theme) => [
    color(Colors.SideBar.foreground.from(theme)),
    marginVertical(2),
    textOverflow(`Ellipsis),
    Style.width(width),
  ];
  let versionText = (~width, ~theme) => [
    color(
      Colors.SideBar.foreground.from(theme)
      |> Revery.Color.multiplyAlpha(0.75),
    ),
    marginTop(4),
    Style.width(width),
  ];

  let text = (~theme) => [
    color(
      Colors.SideBar.foreground.from(theme)
      |> Revery.Color.multiplyAlpha(0.75),
    ),
    marginVertical(2),
    textOverflow(`Ellipsis),
    textWrap(Revery.TextWrapping.NoWrap),
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
  let input = [flexGrow(1), margin(12)];

  let button = (~backgroundColor) => [
    Style.backgroundColor(backgroundColor),
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    flexGrow(0),
  ];
  let innerButton = [margin(2)];
};

module ActionButton = {
  let make =
      (
        ~font: UiFont.t,
        ~title: string,
        ~backgroundColor,
        ~color,
        ~onAction,
        (),
      ) => {
    // TODO
    ignore(color);
    <Sneakable style={Styles.button(~backgroundColor)} onClick=onAction>
      <View style=Styles.innerButton>
        <Text
          fontFamily={font.family}
          fontSize=11.
          fontWeight=Revery.Font.Weight.SemiBold
          text=title
        />
      </View>
    </Sneakable>;
  };
};

let make =
    (
      ~actionButton=React.empty,
      ~width,
      ~iconPath,
      ~theme,
      ~displayName,
      ~author,
      ~version,
      ~font: UiFont.t,
      ~onClick,
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

  <Clickable style={Styles.container(~width)} onClick>
    <View style=Styles.imageContainer> icon </View>
    <View>
      <View
        style=Style.[
          flexDirection(`Row),
          justifyContent(`SpaceBetween),
          width(descriptionWidth),
        ]>
        <Text
          style={Styles.titleText(~width=width - defaultWidth, ~theme)}
          fontFamily={font.family}
          fontSize=13.0
          fontWeight=Revery.Font.Weight.Bold
          text=displayName
        />
        <Text
          style={Styles.versionText(~width=defaultWidth, ~theme)}
          fontFamily={font.family}
          fontSize=10.
          text=version
        />
      </View>
      <View
        style=Style.[
          flexDirection(`Row),
          justifyContent(`SpaceBetween),
          width(descriptionWidth),
        ]>
        <Text
          style={Styles.text(~theme)}
          fontFamily={font.family}
          fontSize={font.size}
          text=author
        />
        actionButton
      </View>
    </View>
  </Clickable>;
};
