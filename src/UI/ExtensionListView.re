open Oni_Core;
open Oni_Model;
open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Extensions;

module Option = Oni_Core.Utility.Option;

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
    ];
};

let make = (~state: State.t, ()) => {
  let extensions =
    Extensions.getExtensions(state.extensions) |> Array.of_list;
  let count = Array.length(extensions);

  let {theme, uiFont, _}: State.t = state;

  let renderItem = idx => {
    let extension = extensions[idx];

    let icon =
      switch (ExtensionManifest.getIcon(extension.manifest)) {
      | None => <Container color=Colors.darkGray width=45 height=45 />
      | Some(iconPath) => <Image src=iconPath width=45 height=45 />
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
      </View>
    </View>;
  };

  <View style=Styles.container>
    <FlatList rowHeight=50 count render=renderItem focused=None />
  </View>;
};
