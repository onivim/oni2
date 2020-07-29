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

let header = (~font: UiFont.t, ~maybeLogo, ~displayName, ~description, ()) => {
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
      ]>
      logo
    </View>
    <column>
      <row>
        <Text
          fontFamily={font.family}
          fontSize=24.
          fontWeight=Revery.Font.Weight.Bold
          text=displayName
        />
      </row>
      <row>
        <Text fontFamily={font.family} fontSize=18. text=description />
      </row>
    </column>
  </View>;
};

let make =
    (~model: Model.model, ~theme, ~tokenTheme, ~font: UiFont.t, ~dispatch, ()) => {
  switch (model.selected) {
  | None =>
    prerr_endline("NOTHING SELECTED");
    <View />;
  | Some(selected) =>
    prerr_endline("SOMETHING SELECTED");
    let maybeLogo = selected |> Selected.logo;
    let displayName = selected |> Selected.displayName;
    let description =
      selected |> Selected.description |> Option.value(~default="");

    <View style=Styles.container>
      <header font maybeLogo displayName description />
      <Oni_Components.Markdown
        colorTheme=theme
        tokenTheme
        markdown="Hello"
        languageInfo=Exthost.LanguageInfo.initial
        grammars=Oni_Syntax.GrammarRepository.empty
        fontFamily={font.family}
        codeFontFamily={font.family}
      />
    </View>;
  };
};
