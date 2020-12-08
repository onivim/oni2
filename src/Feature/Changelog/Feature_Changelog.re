open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2.Feature.Changelog"));

// MODEL

type model = {
  currentMarkdown: string,
  previousMarkdown: string,
};

let initial: model = {
  currentMarkdown: "",
  previousMarkdown: "",
};

[@deriving show({with_path: false})]
type msg =
  | PullRequestClicked(int)
  | CommitHashClicked(string)
  | IssueClicked(int);

let update = (model, msg) =>
  switch (msg) {
  | PullRequestClicked(pr) =>
    let url = Printf.sprintf("https://github.com/onivim/oni2/pull/%d", pr);
    let effect = Service_OS.Effect.openURL(url);
    (model, effect);
  | IssueClicked(issue) =>
    let url =
      Printf.sprintf("https://github.com/onivim/oni2/issues/%d", issue);
    let effect = Service_OS.Effect.openURL(url);
    (model, effect);
  | CommitHashClicked(hash) =>
    let url =
      Printf.sprintf("https://github.com/onivim/oni2/commit/%s", hash);
    let effect = Service_OS.Effect.openURL(url);
    (model, effect);
  };

// READ

let read = () => {


  try({
    let currentMarkdown = 
    Revery.Environment.getAssetPath("CHANGES_CURRENT.md")
    |> Utility.File.readAllLines
    |> String.concat("\n");

    let previousMarkdown = 
    Revery.Environment.getAssetPath("CHANGES.md")
    |> Utility.File.readAllLines
    |> String.concat("\n");

    Ok({
      currentMarkdown,
      previousMarkdown
    })
  }) {
  | exn => 
    Error(Printexc.to_string(exn))
  };
};

// VIEW

let model = Lazy.from_fun(read);

module View = {
  open Revery.UI;
  open Revery.UI.Components;
  open Oni_Components;

  module Colors = Feature_Theme.Colors;

  // STYLES

  module Styles = {
    open Style;

    let typWidth = 60;
    let scopeWidth = 100;
    let caretWidth = 20;
    let breakingChangeIconWidth = 30;

    let scrollContainer = [flexGrow(1)];

    let content = [
      padding(10),
      paddingLeft(20),
      paddingRight(20),
      overflow(`Hidden),
    ];

    let header = (~theme) => [
      color(Colors.foreground.from(theme)),
      marginTop(16),
    ];

    let commit = [flexDirection(`Row), marginTop(10)];

    let groupHeader = (~theme) => [
      color(Colors.foreground.from(theme)),
      marginTop(16),
    ];

    let groupBody = [paddingLeft(10)];

    let typ = (~color) => [Style.color(color), width(typWidth)];

    let scope = (~theme) => [
      width(scopeWidth),
      color(
        Colors.foreground.from(theme) |> Revery.Color.multiplyAlpha(0.75),
      ),
    ];

    let summary = (~theme) => [color(Colors.foreground.from(theme))];

    let breakingCommit = [marginTop(10), marginBottom(6)];

    let breaking = [flexDirection(`Row), marginTop(4)];

    let breakingText = (~theme) => [
      color(Colors.foreground.from(theme)),
      marginLeft(10),
    ];

    let breakingChangeIcon = [width(breakingChangeIconWidth)];

    let caret = [width(caretWidth), marginTop(2)];

    module Full = {
      let titleActive = (~theme) => [
        color(
          Colors.foreground.from(theme) |> Revery.Color.multiplyAlpha(0.75),
        ),
      ];

      let titleInactive = (~theme) => [
        color(Colors.foreground.from(theme)),
      ];
    };
  };

  let title = (~text, ~uiFont: UiFont.t, ~theme, ()) => {
    <Text
      style={Styles.summary(~theme)}
      fontFamily={uiFont.family}
      fontSize=12.
      text
    />;
  };

  // MOREINFO

  // FULL

  module Full = {
    let title = (~text, ~uiFont: UiFont.t, ~theme, ()) => {
      <ClickableText
        activeStyle={Styles.Full.titleActive(~theme)}
        inactiveStyle={Styles.Full.titleInactive(~theme)}
        text
        fontFamily={uiFont.family}
        fontSize=12.
      />;
    };

    let make = (~state: model, ~theme, ~uiFont: UiFont.t, ~dispatch, ()) => {
      switch (Lazy.force(model)) {
      | Ok({currentMarkdown, previousMarkdown}) =>
        <ScrollView style=Styles.scrollContainer>
          <View style=Styles.content>
            <Text
              text="Changelog"
              style={Styles.header(~theme)}
              fontFamily={uiFont.family}
              fontSize=20.
            />
        <Markdown
          headerMargin=16
          colorTheme=theme
          tokenTheme=Oni_Syntax.TokenTheme.empty
          languageInfo=Exthost.LanguageInfo.initial
          grammars=Oni_Syntax.GrammarRepository.empty
          fontFamily=uiFont.family
          codeFontFamily=uiFont.family 
          markdown=currentMarkdown />
        <Markdown
          headerMargin=16
          colorTheme=theme
          tokenTheme=Oni_Syntax.TokenTheme.empty
          languageInfo=Exthost.LanguageInfo.initial
          grammars=Oni_Syntax.GrammarRepository.empty
          fontFamily=uiFont.family
          codeFontFamily=uiFont.family 
          markdown=previousMarkdown />
          </View>
        </ScrollView>
      | Error(message) => <Text text=message />
      };
    };
  };

  // UPDATE

  module Update = {

    let make = (~since, ~theme, ~uiFont: UiFont.t, ()) => {
      switch (Lazy.force(model)) {
      | Ok({currentMarkdown, _}) =>

        <Markdown
          headerMargin=16
          colorTheme=theme
          tokenTheme=Oni_Syntax.TokenTheme.empty
          languageInfo=Exthost.LanguageInfo.initial
          grammars=Oni_Syntax.GrammarRepository.empty
          fontFamily=uiFont.family
          codeFontFamily=uiFont.family 
          markdown=currentMarkdown />

      | Error(message) => <Text text=message />
      };
    };
  };
};
