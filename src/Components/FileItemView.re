open Revery;
open Revery.UI;
open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2.UI.LocationListItem"));

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let clickable = [cursor(Revery.MouseCursors.pointer)];

  let result = [
    height(25),
    flexGrow(1),
    flexDirection(`Row),
    overflow(`Hidden),
    justifyContent(`Center),
    alignItems(`Center),
    paddingHorizontal(8),
  ];

  let snippet = (~theme, ~isHighlighted) => [
    color(
      isHighlighted
        ? Colors.Oni.normalModeBackground.from(theme)
        : Colors.foreground.from(theme),
    ),
    textWrap(TextWrapping.NoWrap),
  ];

  let fileText = (~theme) => [
    color(Colors.foreground.from(theme)),
    textWrap(TextWrapping.NoWrap),
  ];

  let directoryText = (~theme) => [
    color(Colors.List.deemphasizedForeground.from(theme)),
    textOverflow(`Ellipsis),
    textWrap(TextWrapping.NoWrap),
  ];
};

let getFileIcon = (~languageInfo, ~iconTheme, filePath) => {
  Exthost.LanguageInfo.getLanguageFromFilePath(languageInfo, filePath)
  |> IconTheme.getIconForFile(iconTheme, filePath);
};

let setiIcon = (~icon, ~fontSize, ~fg, ()) => {
  <Text
    text={FontIcon.codeToIcon(icon)}
    style=Style.[
      color(fg),
      width(int_of_float(fontSize *. 1.5)),
      height(int_of_float(fontSize *. 1.75)),
      textWrap(TextWrapping.NoWrap),
      // Minor adjustment to center vertically
      marginTop(-2),
      marginLeft(-4),
      paddingLeft(4),
      paddingRight(4),
      flexGrow(0),
      flexShrink(0),
    ]
    fontFamily={Revery.Font.Family.fromFile("seti.ttf")}
    fontSize={fontSize *. 2.}
  />;
};

module View = {
  let make =
      (
        ~theme,
        ~uiFont: UiFont.t,
        ~iconTheme,
        ~languageInfo,
        ~item: string,
        ~workingDirectory,
        (),
      ) => {
    let filename = Path.filename(item);
    let dirname =
      Path.dirname(item |> Path.toRelative(~base=workingDirectory));

    let iconElement =
      getFileIcon(~languageInfo, ~iconTheme, item)
      |> Option.map((icon: IconTheme.IconDefinition.t) => {
           <setiIcon
             fontSize=11.
             fg={icon.fontColor}
             icon={icon.fontCharacter}
           />
         })
      |> Option.value(~default=React.empty);

    <View style=Styles.result>
      iconElement
      <View style=Style.[flexGrow(1), flexShrink(0), marginHorizontal(8)]>
        <Text
          style={Styles.fileText(~theme)}
          text=filename
          fontFamily={uiFont.family}
          fontSize={uiFont.size}
          fontWeight=Revery.Font.Weight.SemiBold
        />
      </View>
      <View style=Style.[flexGrow(0), flexShrink(1)]>
        <Opacity opacity=0.75>
          <Text
            style={Styles.directoryText(~theme)}
            text=dirname
            fontFamily={uiFont.family}
            fontSize={uiFont.size}
            fontWeight=Revery.Font.Weight.Light
          />
        </Opacity>
      </View>
    </View>;
  };
};
