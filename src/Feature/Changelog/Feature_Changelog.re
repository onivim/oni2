open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2.Feature.Changelog"));

// MODEL

[@deriving show({with_path: false})]
type commit = {
  hash: string,
  time: float,
  pr: option(int),
  typ: option(string),
  scope: option(string),
  issue: option(int),
  summary: string,
  breaking: list(string),
};

type model = {expanded: list(commit)};

let initial: model = {expanded: []};

[@deriving show({with_path: false})]
type msg =
  | PullRequestClicked(int)
  | CommitHashClicked(string)
  | IssueClicked(int)
  | ChangeExpanded(commit)
  | ChangeContracted(commit);

type simpleXml =
  | Element(string, list((string, string)), list(simpleXml))
  | Text(string);

let isSameDate = (a, b) => {
  let a = Unix.localtime(a);
  let b = Unix.localtime(b);

  a.tm_year == b.tm_year && a.tm_yday == b.tm_yday;
};

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
  | ChangeExpanded(commit) => (
      {expanded: [commit, ...model.expanded]},
      Isolinear.Effect.none,
    )
  | ChangeContracted(commit) => (
      {expanded: model.expanded |> List.filter(c => c != commit)},
      Isolinear.Effect.none,
    )
  };

// READ

let read = () => {
  open Base.Result.Let_syntax;

  let simplify = stream =>
    stream
    |> Markup.trim
    |> Markup.tree(
         ~text=strings => Text(strings |> String.concat("")),
         ~element=
           ((_ns, name), attrs, children) => {
             let attrs =
               List.map((((_ns, name), value)) => (name, value), attrs);
             Element(name, attrs, children);
           },
       )
    |> Option.get;

  let parseCommit =
    fun
    | Element("commit", attrs, children) => {
        let hash =
          try(List.assoc("hash", attrs)) {
          | Not_found => failwith("hash is required")
          };
        let time =
          try(List.assoc("time", attrs) |> float_of_string) {
          | Not_found => failwith("time is required")
          };
        let pr =
          List.assoc_opt("pr", attrs) |> OptionEx.flatMap(int_of_string_opt);
        let typ = List.assoc_opt("type", attrs);
        let scope = List.assoc_opt("scope", attrs);
        let issue =
          List.assoc_opt("issue", attrs)
          |> OptionEx.flatMap(int_of_string_opt);

        let commit = {
          hash,
          time,
          pr,
          typ,
          scope,
          issue,
          summary: "",
          breaking: [],
        };

        List.fold_left(
          commit =>
            fun
            | Text(summary) => {
                ...commit,
                summary: commit.summary ++ summary,
              }
            | Element("breaking", _, [Text(text)]) => {
                ...commit,
                breaking: [text, ...commit.breaking],
              }
            | Element(name, _, _) => {
                Log.warnf(m => m("Unexpected element %s in summary", name));
                commit;
              },
          commit,
          children,
        );
      }
    | Element(_) => failwith("Unexpected element")
    | Text(_) => failwith("Unexpected text node");

  let parse =
    fun
    | Element("changelog", _, children) => List.map(parseCommit, children)
    | Element(_) => failwith("Unexpected element")
    | Text(_) => failwith("Unexpected text node");

  let path = Revery.Environment.getAssetPath("changelog.xml");

  let%bind (stream, close) =
    try(Markup.file(path) |> Result.ok) {
    | exn => exn |> Printexc.to_string |> Result.error
    };

  let simpleXml = stream |> Markup.parse_xml |> Markup.signals |> simplify;

  close();

  switch (parse(simpleXml)) {
  | commits => Ok(commits)
  | exception (Failure(message)) => Error(message)
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

    let header = (font: UiFont.t, ~theme) => [
      fontFamily(font.fontFile),
      fontSize(20.),
      color(Colors.foreground.from(theme)),
      marginTop(16),
    ];

    let commit = [flexDirection(`Row), marginTop(10)];

    let groupHeader = (font: UiFont.t, ~theme) => [
      fontFamily(font.fontFile),
      fontSize(16.),
      color(Colors.foreground.from(theme)),
      marginTop(16),
    ];

    let groupBody = [paddingLeft(10)];

    let typ = (font: UiFont.t, ~color) => [
      fontFamily(font.fontFile),
      fontSize(12.),
      Style.color(color),
      width(typWidth),
    ];

    let scope = (font: UiFont.t, ~theme) => [
      fontFamily(font.fontFile),
      fontSize(12.),
      width(scopeWidth),
      color(
        Colors.foreground.from(theme) |> Revery.Color.multiplyAlpha(0.75),
      ),
    ];

    let summary = (font: UiFont.t, ~theme) => [
      fontFamily(font.fontFile),
      fontSize(12.),
      color(Colors.foreground.from(theme)),
    ];

    let breakingCommit = [marginTop(10), marginBottom(6)];

    let breaking = [flexDirection(`Row), marginTop(4)];

    let breakingText = (font: UiFont.t, ~theme) => [
      fontFamily(font.fontFile),
      fontSize(12.),
      color(Colors.foreground.from(theme)),
      marginLeft(10),
    ];

    let error = (font: UiFont.t) => [fontFamily(font.fontFile)];

    let breakingChangeIcon = [width(breakingChangeIconWidth)];

    let caret = [width(caretWidth), marginTop(2)];

    module MoreInfo = {
      let main = [
        marginTop(4),
        marginLeft(
          typWidth + scopeWidth + breakingChangeIconWidth + caretWidth,
        ),
        marginBottom(12),
      ];

      let header = (font: UiFont.t, ~theme) => [
        fontFamily(font.fontFile),
        fontSize(12.),
        color(
          Colors.foreground.from(theme) |> Revery.Color.multiplyAlpha(0.75),
        ),
        width(140),
      ];

      let breakingChangesHeader = (font: UiFont.t, ~theme) => [
        fontFamily(font.fontFile),
        fontSize(12.),
        color(
          Colors.EditorWarning.foreground.from(theme)
          |> Revery.Color.multiplyAlpha(0.75),
        ),
        width(140),
      ];

      let body = (font: UiFont.t, ~theme) => [
        fontFamily(font.fontFile),
        fontSize(12.),
        color(Colors.foreground.from(theme)),
      ];

      let linkActive = (font: UiFont.t, ~theme) => [
        fontFamily(font.fontFile),
        fontSize(12.),
        color(Colors.TextLink.activeForeground.from(theme)),
      ];

      let linkInactive = (font: UiFont.t, ~theme) => [
        fontFamily(font.fontFile),
        fontSize(12.),
        color(Colors.TextLink.foreground.from(theme)),
      ];

      let description = [marginVertical(6), flexDirection(`Row)];
    };

    module Full = {
      let titleActive = (font: UiFont.t, ~theme) => [
        fontFamily(font.fontFile),
        fontSize(12.),
        color(
          Colors.foreground.from(theme) |> Revery.Color.multiplyAlpha(0.75),
        ),
      ];

      let titleInactive = (font: UiFont.t, ~theme) => [
        fontFamily(font.fontFile),
        fontSize(12.),
        color(Colors.foreground.from(theme)),
      ];
    };
  };

  let date = (~commit, ~style, ~withTime=false, ()) => {
    let time = Unix.localtime(commit.time);
    let text =
      withTime
        ? Printf.sprintf(
            "%u-%02u-%02u %02u:%02u",
            time.tm_year + 1900,
            time.tm_mon + 1,
            time.tm_mday,
            time.tm_hour,
            time.tm_min,
          )
        : Printf.sprintf(
            "%u-%02u-%02u",
            time.tm_year + 1900,
            time.tm_mon + 1,
            time.tm_mday,
          );
    <Text style text />;
  };

  let typ = (~commit, ~uiFont, ~theme, ()) => {
    let text =
      switch (commit.typ) {
      | Some("feat") => "feature"
      | Some("fix") => "bugfix"
      | Some(other) => other
      | None => "unknown"
      };

    let color =
      switch (commit.typ) {
      | Some("feat") => Colors.Oni.visualModeBackground.from(theme)
      | Some("fix") => Colors.Oni.operatorModeBackground.from(theme)
      | Some("perf") => Colors.Oni.normalModeBackground.from(theme)
      | _ => Colors.foreground.from(theme)
      };

    <Text style={Styles.typ(uiFont, ~color)} text />;
  };

  let scope = (~commit, ~uiFont, ~theme, ()) => {
    let text =
      switch (commit.scope) {
      | Some(scope) => scope
      | None => "other"
      };

    <Text style={Styles.scope(uiFont, ~theme)} text />;
  };

  let title = (~text, ~uiFont, ~theme, ()) => {
    <Text style={Styles.summary(uiFont, ~theme)} text />;
  };

  let breakingChangeIcon = (~commit, ~theme, ()) => {
    <View style=Styles.breakingChangeIcon>
      {commit.breaking != []
         ? <View style=Style.[marginRight(8)]>
             <FontIcon
               icon=FontAwesome.exclamationTriangle
               color={Colors.EditorWarning.foreground.from(theme)}
               fontSize=12.
             />
           </View>
         : React.empty}
    </View>;
  };

  // MOREINFO

  module MoreInfo = {
    let hash = (~commit, ~uiFont, ~theme, ~dispatch, ()) => {
      let text = Printf.sprintf("#%s", commit.hash);
      let onClick = _ => dispatch(CommitHashClicked(commit.hash));

      <View style=Styles.MoreInfo.description>
        <Text text="Commit" style={Styles.MoreInfo.header(uiFont, ~theme)} />
        <ClickableText
          text
          onClick
          activeStyle={Styles.MoreInfo.linkActive(uiFont, ~theme)}
          inactiveStyle={Styles.MoreInfo.linkInactive(uiFont, ~theme)}
        />
      </View>;
    };

    let description = (~commit, ~uiFont, ~theme, ()) => {
      switch (String.index_opt(commit.summary, '\n')) {
      | Some(i) =>
        let text =
          String.sub(commit.summary, i, String.length(commit.summary) - i);
        <View style=Styles.MoreInfo.description>
          <Text
            text="Description"
            style={Styles.MoreInfo.header(uiFont, ~theme)}
          />
          <Text style={Styles.MoreInfo.body(uiFont, ~theme)} text />
        </View>;
      | None => React.empty
      };
    };

    let pullRequest = (~commit, ~uiFont, ~theme, ~dispatch, ()) => {
      switch (commit.pr) {
      | Some(pr) =>
        let text = Printf.sprintf("#%d", pr);
        let onClick = _ => dispatch(PullRequestClicked(pr));

        <View style=Styles.MoreInfo.description>
          <Text
            text="Pull Request"
            style={Styles.MoreInfo.header(uiFont, ~theme)}
          />
          <ClickableText
            text
            onClick
            activeStyle={Styles.MoreInfo.linkActive(uiFont, ~theme)}
            inactiveStyle={Styles.MoreInfo.linkInactive(uiFont, ~theme)}
          />
        </View>;
      | None => React.empty
      };
    };

    let issue = (~commit, ~uiFont, ~theme, ~dispatch, ()) => {
      switch (commit.issue) {
      | Some(issue) =>
        let text = Printf.sprintf("#%d", issue);
        let onClick = _ => dispatch(IssueClicked(issue));

        <View style=Styles.MoreInfo.description>
          <Text text="Issue" style={Styles.MoreInfo.header(uiFont, ~theme)} />
          <ClickableText
            text
            onClick
            activeStyle={Styles.MoreInfo.linkActive(uiFont, ~theme)}
            inactiveStyle={Styles.MoreInfo.linkInactive(uiFont, ~theme)}
          />
        </View>;
      | None => React.empty
      };
    };

    let breakingChanges = (~commit, ~uiFont, ~theme, ()) => {
      switch (commit.breaking) {
      | [] => React.empty
      | changes =>
        let text =
          List.fold_left(
            (acc, change) => "• " ++ change ++ "\n" ++ acc,
            "",
            changes,
          );
        <View style=Styles.MoreInfo.description>
          <Text
            text="Breaking Changes"
            style={Styles.MoreInfo.breakingChangesHeader(uiFont, ~theme)}
          />
          <Text style={Styles.MoreInfo.body(uiFont, ~theme)} text />
        </View>;
      };
    };

    let make = (~commit, ~uiFont, ~theme, ~dispatch, ()) => {
      <View style=Styles.MoreInfo.main>
        <hash commit uiFont theme dispatch />
        <pullRequest commit uiFont theme dispatch />
        <issue commit uiFont theme dispatch />
        <description commit uiFont theme />
        <breakingChanges commit uiFont theme />
      </View>;
    };
  };

  // FULL

  module Full = {
    let title = (~text, ~uiFont, ~theme, ()) => {
      <ClickableText
        activeStyle={Styles.Full.titleActive(uiFont, ~theme)}
        inactiveStyle={Styles.Full.titleInactive(uiFont, ~theme)}
        text
      />;
    };

    let change = (~model, ~commit, ~uiFont, ~theme, ~dispatch, ()) => {
      let isExpanded = model.expanded |> List.mem(commit);
      let summaryText =
        switch (String.index_opt(commit.summary, '\n')) {
        | Some(i) => String.sub(commit.summary, 0, i)
        | None => commit.summary
        };
      let onCaretClick = _e =>
        isExpanded
          ? dispatch(ChangeContracted(commit))
          : dispatch(ChangeExpanded(commit));

      <View>
        <View style=Styles.commit>
          <breakingChangeIcon commit theme />
          <typ commit uiFont theme />
          <scope commit uiFont theme />
          <Clickable style=Style.[flexDirection(`Row)] onClick=onCaretClick>
            <View style=Styles.caret>
              {isExpanded
                 ? <FontIcon
                     icon=FontAwesome.caretDown
                     color={Colors.foreground.from(theme)}
                     fontSize=12.
                   />
                 : <FontIcon
                     icon=FontAwesome.caretRight
                     color={Colors.foreground.from(theme)}
                     fontSize=12.
                   />}
            </View>
            <title text=summaryText uiFont theme />
          </Clickable>
        </View>
        {isExpanded ? <MoreInfo commit uiFont theme dispatch /> : React.empty}
      </View>;
    };

    let group = (~model, ~commits, ~uiFont, ~theme, ~dispatch, ()) => {
      <View>
        <date
          commit={List.hd(commits)}
          style={Styles.groupHeader(uiFont, ~theme)}
        />
        <View style=Styles.groupBody>
          {commits
           |> List.map(commit =>
                <change model commit uiFont theme dispatch />
              )
           |> React.listToElement}
        </View>
      </View>;
    };

    let make = (~state: model, ~theme, ~uiFont, ~dispatch, ()) => {
      let isSignificantCommit = commit =>
        switch (commit.typ) {
        | Some("feat" | "fix" | "perf") => true
        | _ => false
        };

      switch (Lazy.force(model)) {
      | Ok(commits) =>
        <ScrollView style=Styles.scrollContainer>
          <View style=Styles.content>
            <Text text="Changelog" style={Styles.header(uiFont, ~theme)} />
            {commits
             |> List.filter(isSignificantCommit)
             |> Base.List.group(~break=(a, b) =>
                  !isSameDate(a.time, b.time)
                )
             |> List.map(commits =>
                  <group model=state commits uiFont theme dispatch />
                )
             |> React.listToElement}
          </View>
        </ScrollView>
      | Error(message) => <Text style={Styles.error(uiFont)} text=message />
      };
    };
  };

  // UPDATE

  module Update = {
    module Parts = {
      let line = (~commit, ~uiFont, ~theme, ()) => {
        let summaryText =
          switch (String.index_opt(commit.summary, '\n')) {
          | Some(i) => String.sub(commit.summary, 0, i)
          | None => commit.summary
          };

        <View style=Styles.commit>
          <typ commit uiFont theme />
          <scope commit uiFont theme />
          <title text=summaryText uiFont theme />
        </View>;
      };

      // BREAKING

      module Breaking = {
        let breakingChange = (~text, ~uiFont, ~theme, ()) => {
          <View style=Styles.breaking>
            <FontIcon
              icon=FontAwesome.exclamationTriangle
              color={Colors.EditorWarning.foreground.from(theme)}
              fontSize=14.
            />
            <Text text style={Styles.breakingText(uiFont, ~theme)} />
          </View>;
        };

        let commit = (~item, ~uiFont, ~theme, ()) => {
          <View style=Styles.breakingCommit>
            <Text
              text={item.summary |> Base.String.split(~on='\n') |> List.hd}
              style={Styles.summary(uiFont, ~theme)}
            />
            {item.breaking
             |> List.map(text => <breakingChange text uiFont theme />)
             |> React.listToElement}
          </View>;
        };

        let make = (~items, ~uiFont, ~theme, ()) => {
          <View>
            <Text
              text="Breaking Changes"
              style={Styles.groupHeader(uiFont, ~theme)}
            />
            <View style=Styles.groupBody>
              {items
               |> List.map(item => <commit item uiFont theme />)
               |> React.listToElement}
            </View>
          </View>;
        };
      };

      // FEATURES

      module Features = {
        let make = (~items, ~uiFont, ~theme, ()) => {
          <View>
            <Text
              text="Features"
              style={Styles.groupHeader(uiFont, ~theme)}
            />
            <View style=Styles.groupBody>
              {items
               |> List.map(commit => <line commit uiFont theme />)
               |> React.listToElement}
            </View>
          </View>;
        };
      };

      // FIXES

      module Fixes = {
        let make = (~items, ~uiFont, ~theme, ()) => {
          <View>
            <Text
              text="Bugfixes"
              style={Styles.groupHeader(uiFont, ~theme)}
            />
            <View style=Styles.groupBody>
              {items
               |> List.map(commit => <line commit uiFont theme />)
               |> React.listToElement}
            </View>
          </View>;
        };
      };
    };

    // Update.make

    let make = (~since, ~theme, ~uiFont, ()) => {
      switch (Lazy.force(model)) {
      | Ok(commits) =>
        let commits =
          List.fold_right(
            commit =>
              fun
              | None when since == commit.hash => Some([])
              | None => None
              | Some(commits) => Some([commit, ...commits]),
            commits,
            None,
          )
          |> Option.value(~default=commits);
        let breaking = commits |> List.filter(commit => commit.breaking != []);
        let features =
          commits |> List.filter(({typ, _}) => typ == Some("feat"));
        let fixes =
          commits
          |> List.filter(
               fun
               | {typ: Some("fix" | "perf"), _} => true
               | _ => false,
             );

        <ScrollView style=Styles.scrollContainer>
          <View style=Styles.content>
            <Text
              text={"Changes since " ++ since}
              style={Styles.header(uiFont, ~theme)}
            />
            {if (breaking == [] && features == [] && fixes == []) {
               <Text text="Nothing" style={Styles.summary(uiFont, ~theme)} />;
             } else {
               React.listToElement([
                 breaking == []
                   ? React.empty
                   : <Parts.Breaking items=breaking uiFont theme />,
                 features == []
                   ? React.empty
                   : <Parts.Features items=features uiFont theme />,
                 fixes == []
                   ? React.empty : <Parts.Fixes items=fixes uiFont theme />,
               ]);
             }}
          </View>
        </ScrollView>;

      | Error(message) => <Text style={Styles.error(uiFont)} text=message />
      };
    };
  };
};
