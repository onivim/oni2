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
      textOverflow(`Ellipsis),
    ];
};

let make = (~state: State.t, ()) => {
  let bundledExtensions =
    Extensions.getExtensions(
      ~category=ExtensionScanner.Bundled,
      state.extensions,
    )
    |> Array.of_list;

  let userExtensions =
    Extensions.getExtensions(
      ~category=ExtensionScanner.User,
      state.extensions,
    )
    |> Array.of_list;

  //let developmentExtensions =
  //Extensions.getExtensions(~category=ExtensionScanner.Development, state.extensions) |> Array.of_list;

  let bundledCount = Array.length(bundledExtensions);
  let userCount = Array.length(userExtensions);
  //let developmentCount = Array.length(developmentExtensions);

  let {theme, uiFont, _}: State.t = state;

  let renderItem = (extensions: array(ExtensionScanner.t), idx) => {
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

  <View style=Styles.container>
    <Accordion
      title="User"
      rowHeight=50
      count=userCount
      renderItem={renderItem(userExtensions)}
      focused=None
      expanded=true
      theme
      uiFont
    />
    <Accordion
      title="Bundled"
      rowHeight=50
      count=bundledCount
      renderItem={renderItem(bundledExtensions)}
      focused=None
      expanded=true
      theme
      uiFont
    />
  </View>;
};
