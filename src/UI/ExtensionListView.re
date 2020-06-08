open Oni_Core;
open Oni_Model;
open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Components;

open Exthost.Extension;

module Colors = Feature_Theme.Colors;

module Styles = {
  let text = (~theme) =>
    Style.[
      color(Colors.SideBar.foreground.from(theme)),
      marginLeft(10),
      marginVertical(2),
      textWrap(TextWrapping.NoWrap),
      textOverflow(`Ellipsis),
    ];
};

let make = (~model, ~theme, ~font: UiFont.t, ()) => {
  let renderItem = (extensions: array(Scanner.ScanResult.t), idx) => {
    let extension = extensions[idx];

    let icon =
      switch (extension.manifest.icon) {
      | None => <Container color=Revery.Colors.darkGray width=32 height=32 />
      | Some(iconPath) => <Image src=`File(iconPath) width=32 height=32 />
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
          style={Styles.text(~theme)}
          fontFamily={font.normal}
          fontSize={font.size}
          text={Manifest.getDisplayName(extension.manifest)}
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
              fontFamily={font.normal}
              fontSize={font.size}
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
              style={Styles.text(~theme)}
              fontFamily={font.normal}
              fontSize={font.size}
              text={extension.manifest.version}
            />
          </View>
        </View>
      </View>
    </View>;
  };

  let bundledExtensions =
    Extensions.getExtensions(~category=Scanner.Bundled, model);

  let userExtensions =
    Extensions.getExtensions(~category=Scanner.User, model);

  //let developmentExtensions =
  //Extensions.getExtensions(~category=ExtensionScanner.Development, state.extensions) |> Array.of_list;

  let allExtensions = bundledExtensions @ userExtensions |> Array.of_list;
  //let developmentCount = Array.length(developmentExtensions);

  <FlatList
    rowHeight=50 count={Array.length(allExtensions)} focused=None theme>
    ...{renderItem(allExtensions)}
  </FlatList>;
};
