open Oni_Core;
open Oni_Model;
open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Components;

open Oni_Extensions;

module Colors = Feature_Theme.Colors;

module Styles = {
  let text = (~theme, ~font: UiFont.t) =>
    Style.[
      fontSize(font.fontSize),
      fontFamily(font.fontFile),
      color(Colors.SideBar.foreground.from(theme)),
      marginLeft(10),
      marginVertical(2),
      textWrap(TextWrapping.NoWrap),
      textOverflow(`Ellipsis),
    ];
};

let make = (~extensions, ~theme, ~font, ()) => {
  let renderItem = (extensions: array(ExtensionScanner.t), idx) => {
    let extension = extensions[idx];

    let icon =
      switch (extension.manifest.icon) {
      | None => <Container color=Revery.Colors.darkGray width=32 height=32 />
      | Some(iconPath) => <Image src=iconPath width=32 height=32 />
      };

    <View
      style=Style.[
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
        flexGrow(1),
        height(45),
      ]>
      icon
      <View style=Style.[flexDirection(`Column), flexGrow(1)]>
        <Text
          style={Styles.text(~font, ~theme)}
          text={ExtensionManifest.getDisplayName(extension.manifest)}
        />
        <View style=Style.[flexDirection(`Row), flexGrow(1)]>
          <View
            style=Style.[
              flexDirection(`Column),
              flexGrow(1),
              overflow(`Hidden),
            ]>
            <Text
              style={Styles.text(~font, ~theme)}
              text={extension.manifest.author}
            />
          </View>
          <View
            style=Style.[
              flexDirection(`Column),
              flexGrow(1),
              overflow(`Hidden),
            ]>
            <Text
              style={Styles.text(~font, ~theme)}
              text={extension.manifest.version}
            />
          </View>
        </View>
      </View>
    </View>;
  };

  let bundledExtensions =
    Extensions.getExtensions(~category=ExtensionScanner.Bundled, extensions);

  let userExtensions =
    Extensions.getExtensions(~category=ExtensionScanner.User, extensions);

  //let developmentExtensions =
  //Extensions.getExtensions(~category=ExtensionScanner.Development, state.extensions) |> Array.of_list;

  let allExtensions = bundledExtensions @ userExtensions |> Array.of_list;
  //let developmentCount = Array.length(developmentExtensions);

  <FlatList rowHeight=50 count={Array.length(allExtensions)} focused=None>
    ...{renderItem(allExtensions)}
  </FlatList>;
};
