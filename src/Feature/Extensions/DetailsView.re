open Oni_Core;
open Revery.UI;
open Revery.UI.Components;
open Oni_Components;

open Model;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;
  let container = [flexDirection(`Column), flexGrow(1), overflow(`Hidden)];

  let row = [
    flexDirection(`Row),
    flexGrow(1),
    flexShrink(0),
    justifyContent(`Center),
    alignItems(`Center),
  ];

  let column = [
    flexDirection(`Column),
    flexGrow(1),
    justifyContent(`Center),
    alignItems(`Center),
  ];

  let headerRow = [
    flexGrow(0),
    flexDirection(`Row),
    alignItems(`Center),
    justifyContent(`FlexStart),
    marginLeft(8),
  ];

  let headerTextContainer = [marginHorizontal(8), marginVertical(4)];
};

let installButton = (~font, ~extensionId, ~dispatch, ()) => {
  <ItemView.ActionButton
    font
    title="Install"
    backgroundColor=Revery.Colors.green
    color=Revery.Colors.white
    onAction={() =>
      dispatch(Model.InstallExtensionClicked({extensionId: extensionId}))
    }
  />;
};

let header =
    (
      ~font: UiFont.t,
      ~maybeLogo,
      ~displayName,
      ~description,
      ~extensionId,
      ~version,
      (),
    ) => {
  let logo =
    switch (maybeLogo) {
    | Some(src) => <Image width=96 height=96 src={`File(src)} />
    // TODO: Replace with real logo
    | None => <Container color=Revery.Colors.gray height=96 width=96 />
    };

  <View
    style=Style.[
      flexDirection(`Row),
      flexGrow(0),
      flexShrink(0),
      flexBasis(200),
      backgroundColor(Revery.Color.rgba_int(0, 0, 0, 32)),
    ]>
    <View
      style=Style.[
        flexDirection(`Column),
        justifyContent(`Center),
        alignItems(`Center),
        margin(8),
      ]>
      logo
    </View>
    <View style=Style.[flexDirection(`Column), justifyContent(`Center)]>
      <View style=Styles.headerRow>
        <View style=Styles.headerTextContainer>
          <Text
            fontFamily={font.family}
            fontSize=24.
            fontWeight=Revery.Font.Weight.Bold
            text=displayName
          />
        </View>
      </View>
      <View style=Styles.headerRow>
        <View
          style=Style.[
            backgroundColor(Revery.Color.rgba_int(128, 128, 128, 64)),
            marginLeft(8),
            padding(4),
          ]>
          <Text
            fontFamily={font.family}
            fontSize=18.
            fontWeight=Revery.Font.Weight.Bold
            text=extensionId
          />
        </View>
        <View style=Styles.headerTextContainer>
          <Text fontFamily={font.family} fontSize=18. text=version />
        </View>
      </View>
      <View style=Styles.headerRow>
        <View style=Styles.headerTextContainer>
          <Text fontFamily={font.family} fontSize=18. text=description />
        </View>
      </View>
    </View>
  </View>;
};

let make =
    (
      ~model: Model.model,
      ~theme,
      ~tokenTheme,
      ~font: UiFont.t,
      ~dispatch as _,
      (),
    ) => {
  switch (model.selected) {
  | None => <View />
  | Some(selected) =>
    let maybeLogo = selected |> Selected.logo;
    let displayName = selected |> Selected.displayName;
    let description =
      selected |> Selected.description |> Option.value(~default="");

    let readmeUrl = selected |> Selected.readme;
    let extensionId = selected |> Selected.identifier;
    let version = selected |> Selected.version;

    <View style=Styles.container>
      <header font maybeLogo displayName description extensionId version />
      <ScrollView style=Style.[paddingLeft(64), flexGrow(1)]>
        <RemoteMarkdown
          url=readmeUrl
          colorTheme=theme
          tokenTheme
          languageInfo=Exthost.LanguageInfo.initial
          grammars=Oni_Syntax.GrammarRepository.empty
          fontFamily={font.family}
          codeFontFamily={font.family}
        />
      </ScrollView>
    </View>;
  };
};
