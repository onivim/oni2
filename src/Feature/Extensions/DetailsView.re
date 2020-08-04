open Oni_Core;
open Revery.UI;
open Revery.UI.Components;
open Oni_Components;

open Exthost.Extension;
open Model;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;
  let container = [
    flexDirection(`Column),
    flexGrow(1),
    overflow(`Hidden),
    backgroundColor(Revery.Colors.red),
  ];

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

let row = (~children, ()) => {
  <View style=Styles.row />;
};

let column = (~children, ()) => {
  <View style=Styles.column />;
};

let header =
    (
      ~font: UiFont.t,
      ~maybeLogo,
      ~displayName,
      ~description,
      ~extensionId,
      (),
    ) => {
  let logo =
    switch (maybeLogo) {
    | Some(src) => <Image width=128 height=128 src={`File(src)} />
    // TODO: Replace with real logo
    | None => <Container color=Revery.Colors.gray height=128 width=128 />
    };

  <View
    style=Style.[
      flexDirection(`Row),
      flexGrow(0),
      flexShrink(0),
      flexBasis(200),
      backgroundColor(Revery.Colors.blue),
    ]>
    <View
      style=Style.[
        flexDirection(`Column),
        justifyContent(`Center),
        alignItems(`Center),
        backgroundColor(Revery.Colors.green),
        margin(8),
      ]>
      logo
    </View>
    <View
      style=Style.[
        flexDirection(`Column),
        justifyContent(`Center),
        flexShrink(0),
        flexGrow(1),
      ]>
      <View
        style=Style.[
          flexDirection(`Row),
          alignItems(`Center),
          justifyContent(`FlexStart),
          margin(8),
          backgroundColor(Revery.Colors.aqua),
        ]>
        <Text
          fontFamily={font.family}
          fontSize=24.
          fontWeight=Revery.Font.Weight.Bold
          text=displayName
        />
        <Text
          fontFamily={font.family}
          fontSize=18.
          fontWeight=Revery.Font.Weight.Bold
          text=extensionId
        />
      </View>
      <View
        style=Style.[
          flexDirection(`Row),
          alignItems(`Center),
          justifyContent(`FlexStart),
          margin(8),
          backgroundColor(Revery.Colors.purple),
        ]>
        <Text fontFamily={font.family} fontSize=18. text=description />
      </View>
    </View>
  </View>;
};

let make =
    (~model: Model.model, ~theme, ~tokenTheme, ~font: UiFont.t, ~dispatch, ()) => {
  switch (model.selected) {
  | None => <View />
  | Some(selected) =>
    let maybeLogo = selected |> Selected.logo;
    let displayName = selected |> Selected.displayName;
    let description =
      selected |> Selected.description |> Option.value(~default="");

    let readmeUrl = selected |> Selected.readme;
    let extensionId = selected |> Selected.identifier;

    <View style=Styles.container>
      <header font maybeLogo displayName description extensionId />
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
