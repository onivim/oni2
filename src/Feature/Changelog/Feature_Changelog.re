open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Feature.Changelog"));

// MODEL

type commit = {
  hash: string,
  time: float,
  pr: option(int),
  typ: option(string),
  scope: option(string),
  summary: string,
  breaking: list(string),
};

type model =
  | NotLoaded
  | Loaded(list(commit))
  | Error(string);

type simpleXml =
  | Element(string, list((string, string)), list(simpleXml))
  | Text(string);

let isSameDate = (a, b) => {
  let a = Unix.localtime(a);
  let b = Unix.localtime(b);

  a.tm_year == b.tm_year && a.tm_yday == b.tm_yday;
};

// READ

let read = () => {
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
        let pr = List.assoc_opt("pr", attrs) |> Option.map(int_of_string);
        let typ = List.assoc_opt("type", attrs);
        let scope = List.assoc_opt("scope", attrs);

        let commit = {hash, time, pr, typ, scope, summary: "", breaking: []};

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
  let simpleXml =
    Markup.file(path) |> fst |> Markup.parse_xml |> Markup.signals |> simplify;

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

    let scrollContainer = [flexGrow(1)];

    let content = [
      padding(10),
      paddingLeft(20),
      paddingRight(20),
      overflow(`Hidden),
    ];

    let commit = [flexDirection(`Row), marginTop(6)];

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
      width(60),
    ];

    let scope = (font: UiFont.t, ~theme) => [
      fontFamily(font.fontFile),
      fontSize(12.),
      width(130),
      color(
        Colors.foreground.from(theme) |> Revery.Color.multiplyAlpha(0.75),
      ),
    ];

    let summary = (font: UiFont.t, ~theme) => [
      fontFamily(font.fontFile),
      fontSize(12.),
      color(Colors.foreground.from(theme)),
    ];

    let breaking = [flexDirection(`Row), marginTop(4)];

    let breakingText = (font: UiFont.t, ~theme) => [
      fontFamily(font.fontFile),
      fontSize(12.),
      color(Colors.foreground.from(theme)),
      marginLeft(10),
    ];

    let error = (font: UiFont.t) => [fontFamily(font.fontFile)];
  };

  let date = (~commit, ~style, ()) => {
    let time = Unix.localtime(commit.time);
    let text =
      Printf.sprintf(
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

  let summary = (~commit, ~uiFont, ~theme, ()) => {
    <Text style={Styles.summary(uiFont, ~theme)} text={commit.summary} />;
  };

  // FULL

  module Full = {
    let line = (~commit, ~uiFont, ~theme, ()) => {
      <View style=Styles.commit>
        <typ commit uiFont theme />
        <scope commit uiFont theme />
        <summary commit uiFont theme />
      </View>;
    };

    let group = (~commits, ~uiFont, ~theme, ()) => {
      <View>
        <date
          commit={List.hd(commits)}
          style={Styles.groupHeader(uiFont, ~theme)}
        />
        <View style=Styles.groupBody>
          {commits
           |> List.map(commit => <line commit uiFont theme />)
           |> React.listToElement}
        </View>
      </View>;
    };

    let make = (~theme, ~uiFont, ()) => {
      let isSignificantCommit = commit =>
        switch (commit.typ) {
        | Some("feat" | "fix" | "perf") => true
        | _ => false
        };

      switch (Lazy.force(model)) {
      | Ok(commits) =>
        <ScrollView style=Styles.scrollContainer>
          <View style=Styles.content>
            {commits
             |> List.filter(isSignificantCommit)
             |> Base.List.group(~break=(a, b) =>
                  !isSameDate(a.time, b.time)
                )
             |> List.map(commits => <group commits uiFont theme />)
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
        <View style=Styles.commit>
          <typ commit uiFont theme />
          <scope commit uiFont theme />
          <summary commit uiFont theme />
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
          <View>
            <Text
              text={item.summary}
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

    let make = (~theme, ~uiFont, ()) => {
      switch (Lazy.force(model)) {
      | Ok(commits) =>
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
            <Parts.Breaking items=breaking uiFont theme />
            <Parts.Features items=features uiFont theme />
            <Parts.Fixes items=fixes uiFont theme />
          </View>
        </ScrollView>;

      | Error(message) => <Text style={Styles.error(uiFont)} text=message />
      };
    };
  };
};
