open Oni_Core;
open Revery;
open Revery.UI;

let getFileIcon = (~languageInfo, ~iconTheme, filePath) => {
  Exthost.LanguageInfo.getLanguageFromFilePath(languageInfo, filePath)
  |> Oni_Core.IconTheme.getIconForFile(iconTheme, filePath);
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
      marginRight(10),
    ]
    fontFamily={Revery.Font.Family.fromFile("seti.ttf")}
    fontSize={fontSize *. 2.}
  />;
};

let make = (~font: UiFont.t, ~iconTheme, ~languageInfo, ~path, ()) => {
  switch (getFileIcon(~iconTheme, ~languageInfo, path)) {
  | None => React.empty
  | Some(icon: IconTheme.IconDefinition.t) =>
    <setiIcon
      fontSize={font.size}
      fg={icon.fontColor}
      icon={icon.fontCharacter}
    />
  };
};
