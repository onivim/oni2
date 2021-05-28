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

  let warningRow = [
    flexGrow(0),
    flexDirection(`Row),
    alignItems(`Center),
    justifyContent(`FlexStart),
  ];

  let headerButton = [padding(8)];

  let headerTextContainer = [marginHorizontal(8), marginVertical(4)];
};

module Rating = {
  let make =
      (~color, ~rating: int, ~font: UiFont.t, ~fontSize, ~ratingCount, ()) => {
    let stars = List.init(5, i => i);
    let elems =
      stars
      |> List.map(idx => {
           let icon = idx < rating ? Codicon.starFull : Codicon.starEmpty;
           <Codicon icon fontSize=16. color />;
         })
      |> React.listToElement;

    let foregroundColor = color;
    <View style=Styles.[Style.flexDirection(`Row), ...headerTextContainer]>
      elems
      <View style=Style.[paddingLeft(2), paddingTop(2)]>
        <Text
          style=[Style.color(foregroundColor)]
          fontFamily={font.family}
          fontSize
          text={"(" ++ string_of_int(ratingCount) ++ ")"}
        />
      </View>
    </View>;
  };
};

let installButton = (~theme, ~font, ~extensionId, ~dispatch, ()) => {
  let backgroundColor = Colors.Button.background.from(theme);
  let color = Colors.Button.foreground.from(theme);
  <View style=Styles.headerButton>
    <ItemView.ActionButton
      font
      title="Install"
      extensionId
      backgroundColor
      color
      onAction={() =>
        dispatch(Model.InstallExtensionClicked({extensionId: extensionId}))
      }
    />
  </View>;
};

let uninstallButton = (~theme, ~font, ~extensionId, ~dispatch, ()) => {
  let backgroundColor = Colors.Button.background.from(theme);
  let color = Colors.Button.foreground.from(theme);
  <View style=Styles.headerButton>
    <ItemView.ActionButton
      font
      title="Uninstall"
      extensionId
      backgroundColor
      color
      onAction={() =>
        dispatch(Model.UninstallExtensionClicked({extensionId: extensionId}))
      }
    />
  </View>;
};

let updateButton = (~theme, ~font, ~extensionId, ~dispatch, ()) => {
  let backgroundColor = Colors.Button.secondaryBackground.from(theme);
  let color = Colors.Button.secondaryForeground.from(theme);
  <View style=Styles.headerButton>
    <ItemView.ActionButton
      font
      title="Update"
      extensionId
      backgroundColor
      color
      onAction={() =>
        dispatch(Model.UpdateExtensionClicked({extensionId: extensionId}))
      }
    />
  </View>;
};

let setThemeButton = (~theme, ~font, ~extensionId, ~dispatch, ()) => {
  let backgroundColor = Colors.Button.secondaryBackground.from(theme);
  let color = Colors.Button.secondaryForeground.from(theme);
  <View style=Styles.headerButton>
    <ItemView.ActionButton
      font
      title="Set Theme"
      extensionId
      backgroundColor
      color
      onAction={() =>
        dispatch(Model.SetThemeClicked({extensionId: extensionId}))
      }
    />
  </View>;
};

let header =
    (
      ~model: Model.model,
      ~proxy,
      ~font: UiFont.t,
      ~publisher,
      ~maybeLogo,
      ~displayName,
      ~description,
      ~extensionId,
      ~version,
      ~dispatch,
      ~theme,
      ~maybeRating,
      (),
    ) => {
  let logo =
    switch (maybeLogo) {
    | Some(src) => <RemoteImage proxy width=96 height=96 url=src />
    // TODO: Replace with real logo
    | None => <Container color=Revery.Colors.gray height=96 width=96 />
    };

  let isInstalled = Model.isInstalled(~extensionId, model);
  let hasThemes = Model.hasThemes(~extensionId, model);

  let buttons =
    switch (isInstalled, hasThemes) {
    | (true, true) => [
        <setThemeButton theme font extensionId dispatch />,
        <uninstallButton theme font extensionId dispatch />,
      ]
    | (true, false) => [<uninstallButton theme font extensionId dispatch />]
    | _ => [<installButton theme font extensionId dispatch />]
    };

  // Tack on an update button, if available...
  let buttons' =
    if (isInstalled && Model.isUpdateAvailable(~extensionId, model)) {
      [<updateButton theme font extensionId dispatch />, ...buttons];
    } else {
      buttons;
    };

  let color = Feature_Theme.Colors.foreground.from(theme);
  let fg = color;

  let buttonElements = buttons' |> React.listToElement;
  let headerText = (~fontWeight=Revery.Font.Weight.Normal, ~text, ()) => {
    <View style=Styles.headerTextContainer>
      <Text
        style=Style.[color(fg)]
        fontFamily={font.family}
        fontSize=14.
        fontWeight
        text
      />
    </View>;
  };

  let headerTextSeparator = () => {
    <headerText text="|" />;
  };

  let maybeRatingElement =
    maybeRating
    |> Option.map(({rating, ratingCount, downloadCount}: Model.RatingInfo.t) => {
         <View style=Style.[flexDirection(`Row), alignItems(`Center)]>
           <View style=Style.[marginLeft(4), marginTop(-4)]>
             <Codicon icon=Codicon.cloudDownload fontSize=16. color />
           </View>
           <headerText text={string_of_int(downloadCount)} />
           <headerTextSeparator />
           <Rating rating font fontSize=14. ratingCount color />
         </View>
       })
    |> Option.value(~default=React.empty);

  <View
    style=Style.[
      flexDirection(`Row),
      flexGrow(0),
      flexShrink(0),
      flexBasis(200),
      backgroundColor(Revery.Color.rgba_int(0, 0, 0, 32)),
      boxShadow(
        ~xOffset=4.,
        ~yOffset=4.,
        ~blurRadius=12.,
        ~spreadRadius=0.,
        ~color=Revery.Color.rgba(0., 0., 0., 0.75),
      ),
    ]>
    <View
      style=Style.[
        flexDirection(`Column),
        justifyContent(`Center),
        alignItems(`Center),
        margin(8),
        paddingLeft(12),
      ]>
      logo
    </View>
    <View style=Style.[flexDirection(`Column), justifyContent(`Center)]>
      <View style=Styles.headerRow>
        <View style=Styles.headerTextContainer>
          <Text
            style=Style.[color(fg)]
            fontFamily={font.family}
            fontSize=24.
            fontWeight=Revery.Font.Weight.Bold
            text=displayName
          />
        </View>
        <View
          style=Style.[
            backgroundColor(Revery.Color.rgba_int(128, 128, 128, 64)),
            marginTop(-4),
            marginLeft(8),
            padding(4),
          ]>
          <View style=Style.[marginTop(4)]>
            <Text
              style=Style.[color(fg)]
              fontFamily={font.family}
              fontSize=16.
              fontWeight=Revery.Font.Weight.Bold
              text=extensionId
            />
          </View>
        </View>
      </View>
      <View style=Styles.headerRow>
        <headerText text=publisher />
        <headerTextSeparator />
        <headerText text=version />
        <headerTextSeparator />
        maybeRatingElement
      </View>
      <View style=Styles.headerRow>
        <headerText fontWeight=Revery.Font.Weight.SemiBold text=description />
      </View>
      <View style=Styles.headerRow> buttonElements </View>
    </View>
  </View>;
};

let warning = (~displayName, ~namespace, ~font: UiFont.t, ()) => {
  let bg = Revery.Color.hex("#FFDD57");
  let foregroundColor = Revery.Color.hex("#333");
  <View style=Styles.warningRow>
    <View
      style=Style.[
        backgroundColor(bg),
        flexGrow(1),
        padding(8),
        flexDirection(`Row),
      ]>
      <View
        style=Style.[
          flexShrink(0),
          flexGrow(0),
          justifyContent(`Center),
          alignItems(`Center),
          marginHorizontal(8),
        ]>
        <Codicon
          icon=Codicon.warning
          fontSize=16.
          color={Revery.Color.hex("#333")}
        />
      </View>
      <View style=Style.[flexGrow(1), flexShrink(1)]>
        <Text
          fontFamily={font.family}
          fontSize=12.
          fontWeight=Revery.Font.Weight.Bold
          style=[Style.color(foregroundColor)]
          text={Printf.sprintf(
            "The namespace %s is public, which means anyone can publish a new version of the %s extension. Please verify you trust the source before installing.",
            namespace,
            displayName,
          )}
        />
      </View>
      <View
        style=Style.[
          flexShrink(0),
          flexGrow(0),
          justifyContent(`Center),
          alignItems(`Center),
        ]>
        <Revery.UI.Components.Link
          fontFamily={font.family}
          fontSize=12.
          fontWeight=Revery.Font.Weight.Bold
          inactiveStyle=Style.[
            marginHorizontal(8),
            color(foregroundColor),
            opacity(0.9),
          ]
          activeStyle=Style.[
            marginHorizontal(8),
            color(foregroundColor),
            opacity(1.0),
          ]
          text="More info"
          href="https://github.com/eclipse/openvsx/wiki/Namespace-Access#why-is-a-warning-shown"
        />
      </View>
    </View>
  </View>;
};

let make =
    (
      ~model: Model.model,
      ~proxy,
      ~theme,
      ~tokenTheme,
      ~font: UiFont.t,
      ~dispatch,
      (),
    ) => {
  switch (model.selected) {
  | None => <View />
  | Some(selected) =>
    let maybeLogo = selected |> Selected.logo;
    let displayName = selected |> Selected.displayName;
    let namespace = selected |> Selected.namespace;
    let description =
      selected |> Selected.description |> Option.value(~default="");

    let maybeReadmeUrl = selected |> Selected.readme;
    let extensionId = selected |> Selected.identifier;
    let version =
      selected
      |> Selected.version
      |> Option.map(Semver.to_string)
      |> Option.value(~default="0.0.0");

    let maybeRating = selected |> Selected.ratings;

    let isPublicNamespace = selected |> Selected.isPublicNamespace;

    let warningElement =
      isPublicNamespace ? <warning displayName namespace font /> : React.empty;

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
            proxy
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
        proxy
        theme
        font
        maybeLogo
        publisher=namespace
        displayName
        description
        extensionId
        version
        dispatch
        model
        maybeRating
      />
      warningElement
      contents
    </View>;
  };
};
