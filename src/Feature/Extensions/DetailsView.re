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

  let headerButton = [padding(8)];

  let headerTextContainer = [marginHorizontal(8), marginVertical(4)];
};

let installButton = (~font, ~extensionId, ~dispatch, ()) => {
  <View style=Styles.headerButton>
    <ItemView.ActionButton
      font
      title="Install"
      extensionId
      backgroundColor=Revery.Colors.green
      color=Revery.Colors.white
      onAction={() =>
        dispatch(Model.InstallExtensionClicked({extensionId: extensionId}))
      }
    />
  </View>;
};

let uninstallButton = (~font, ~extensionId, ~dispatch, ()) => {
  <View style=Styles.headerButton>
    <ItemView.ActionButton
      font
      title="Uninstall"
      extensionId
      backgroundColor=Revery.Colors.green
      color=Revery.Colors.white
      onAction={() =>
        dispatch(Model.UninstallExtensionClicked({extensionId: extensionId}))
      }
    />
  </View>;
};

let setThemeButton = (~font, ~extensionId, ~dispatch, ()) => {
  <View style=Styles.headerButton>
    <ItemView.ActionButton
      font
      title="Set Theme"
      extensionId
      backgroundColor=Revery.Colors.green
      color=Revery.Colors.white
      onAction={() =>
        dispatch(Model.SetThemeClicked({extensionId: extensionId}))
      }
    />
  </View>;
};

let header =
    (
      ~model: Model.model,
      ~font: UiFont.t,
      ~maybeLogo,
      ~displayName,
      ~description,
      ~extensionId,
      ~version,
      ~dispatch,
      (),
    ) => {
  let logo =
    switch (maybeLogo) {
    | Some(src) => <Image width=96 height=96 src={`File(src)} />
    // TODO: Replace with real logo
    | None => <Container color=Revery.Colors.gray height=96 width=96 />
    };

  let isInstalled = Model.isInstalled(~extensionId, model);
  let hasThemes = Model.hasThemes(~extensionId, model);

  let buttons =
    (
      switch (isInstalled, hasThemes) {
      | (true, true) => [
          <setThemeButton font extensionId dispatch />,
          <uninstallButton font extensionId dispatch />,
        ]
      | (true, false) => [<uninstallButton font extensionId dispatch />]
      | _ => [<installButton font extensionId dispatch />]
      }
    )
    |> React.listToElement;

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
      <View style=Styles.headerRow> buttons </View>
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

    let maybeReadmeUrl = selected |> Selected.readme;
    let extensionId = selected |> Selected.identifier;
    let version = selected |> Selected.version;

    let contents =
      switch (maybeReadmeUrl) {
      | None =>
        <View
          style=Style.[
            flexGrow(1),
            justifyContent(`Center),
            alignItems(`Center),
          ]>
          <Text
            fontFamily={font.family}
            fontSize=18.
            text="No README available"
          />
        </View>
      | Some(readmeUrl) =>
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
      };

    <View style=Styles.container>
      <header
        font
        maybeLogo
        displayName
        description
        extensionId
        version
        dispatch
        model
      />
      contents
    </View>;
  };
};
