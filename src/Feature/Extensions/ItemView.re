open Oni_Core;
open Revery.UI;

module Colors = Feature_Theme.Colors;
module Sneakable = Feature_Sneak.View.Sneakable;

module Constants = {
  let itemHeight = 72;
  let imageSize = 40;
  let imageContainerSize = 50;
  let defaultIcon = "https://open-vsx.org/default-icon.png";
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
  let titleText = (~theme) => [
    color(Colors.SideBar.foreground.from(theme)),
    marginVertical(2),
    overflow(`Hidden),
    textWrap(Revery.TextWrapping.NoWrap),
    textOverflow(`Ellipsis),
  ];
  let versionText = (~theme) => [
    color(
      Colors.SideBar.foreground.from(theme)
      |> Revery.Color.multiplyAlpha(0.75),
    ),
    marginTop(4),
  ];

  let text = (~theme) => [
    color(
      Colors.SideBar.foreground.from(theme)
      |> Revery.Color.multiplyAlpha(0.75),
    ),
    marginVertical(2),
    textWrap(Revery.TextWrapping.NoWrap),
    textOverflow(`Ellipsis),
  ];

  let restartText = (~theme) => [
    color(Colors.EditorError.foreground.from(theme)),
    marginVertical(2),
    // TODO: Workaround for #2140
    //textOverflow(`Ellipsis),
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
    flexShrink(0),
  ];
  let innerButton = [margin(4)];
};

module ActionButton = {
  let make =
      (
        ~extensionId: string,
        ~font: UiFont.t,
        ~title: string,
        ~backgroundColor,
        ~color,
        ~onAction,
        (),
      ) => {
    // TODO
    ignore(color);
    <Sneakable
      sneakId=extensionId
      style={Styles.button(~backgroundColor)}
      onClick=onAction>
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
      ~isRestartRequired,
      ~version,
      ~font: UiFont.t,
      ~proxy,
      ~showIcon=true,
      ~onClick,
      (),
    ) => {
  let url = iconPath |> Option.value(~default=Constants.defaultIcon);
  let padding =
    <View style=Style.[flexGrow(0), flexShrink(0), width(16)] />;
  let icon =
    showIcon
      ? <View style=Styles.imageContainer>
          <Oni_Components.RemoteImage
            url
            proxy
            width=Constants.imageSize
            height=Constants.imageSize
          />
        </View>
      : React.empty;

  let descriptionWidth =
    showIcon ? width - Constants.imageContainerSize - 16 : width - 32;

  let authorOrRestart =
    isRestartRequired
      ? <Text
          style={Styles.restartText(~theme)}
          fontFamily={font.family}
          fontSize={font.size}
          text="Restart Required"
          italic=true
        />
      : <Text
          style={Styles.text(~theme)}
          fontFamily={font.family}
          fontSize={font.size}
          text=author
        />;

  <Sneakable sneakId=displayName style={Styles.container(~width)} onClick>
    padding
    icon
    <View style=Style.[flexDirection(`Column), width(descriptionWidth)]>
      <View
        style=Style.[flexDirection(`Row), justifyContent(`SpaceBetween)]>
        <View style=Style.[flexGrow(1), flexShrink(1), overflow(`Hidden)]>
          <Text
            style={Styles.titleText(~theme)}
            fontFamily={font.family}
            fontSize=13.0
            fontWeight=Revery.Font.Weight.Bold
            text=displayName
          />
        </View>
        <View style=Style.[flexShrink(0)]>
          <Text
            style={Styles.versionText(~theme)}
            fontFamily={font.family}
            fontSize=10.
            text=version
          />
        </View>
      </View>
      <View
        style=Style.[flexDirection(`Row), justifyContent(`SpaceBetween)]>
        <View style=Style.[flexShrink(1)]> authorOrRestart </View>
        <View style=Style.[flexShrink(0)]> actionButton </View>
      </View>
    </View>
    padding
  </Sneakable>;
};
