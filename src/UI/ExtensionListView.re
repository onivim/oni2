open Oni_Core;
open Oni_Model;
open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Components;

open Oni_Extensions;

module Styles = {
  let container = Style.[flexGrow(1)];

  let text = (~theme: Theme.t, ~font: UiFont.t) =>
    Style.[
      fontSize(font.fontSize),
      fontFamily(font.fontFile),
      color(theme.sideBarForeground),
      marginLeft(10),
      marginVertical(2),
      textWrap(TextWrapping.NoWrap),
      textOverflow(`Ellipsis),
    ];
};

let make = (~state: State.t, ()) => {
  let {theme, uiFont, _}: State.t = state;

  let renderItem = (extensions: array(ExtensionScanner.t), idx) => {
    let extension = extensions[idx];

    let icon =
      switch (ExtensionManifest.getIcon(extension.manifest)) {
      | None => <Container color=Colors.darkGray width=32 height=32 />
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
          style={Styles.text(~font=uiFont, ~theme)}
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
              style={Styles.text(~font=uiFont, ~theme)}
              text={ExtensionManifest.getAuthor(extension.manifest)}
            />
          </View>
          <View
            style=Style.[
              flexDirection(`Column),
              flexGrow(1),
              overflow(`Hidden),
            ]>
            <Text
              style={Styles.text(~font=uiFont, ~theme)}
              text={ExtensionManifest.getVersion(extension.manifest)}
            />
          </View>
        </View>
      </View>
    </View>;
  };

  let bundledExtensions =
    Extensions.getExtensions(
      ~category=ExtensionScanner.Bundled,
      state.extensions,
    );

  let userExtensions =
    Extensions.getExtensions(
      ~category=ExtensionScanner.User,
      state.extensions,
    );

  //let developmentExtensions =
  //Extensions.getExtensions(~category=ExtensionScanner.Development, state.extensions) |> Array.of_list;

  let allExtensions = bundledExtensions @ userExtensions |> Array.of_list;
  //let developmentCount = Array.length(developmentExtensions);

  <View style=Styles.container>
    <FlatList rowHeight=50 count={Array.length(allExtensions)} focused=None>
      ...{renderItem(allExtensions)}
    </FlatList>
  </View>;
};
