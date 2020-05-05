open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Feature.Changelog"));

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

let isSameDate = (a, b) => {
  let a = Unix.localtime(a);
  let b = Unix.localtime(b);

  a.tm_year == b.tm_year && a.tm_yday == b.tm_yday;
};

module View = {
  open Revery.UI;
  open Revery.UI.Components;

  module Colors = Feature_Theme.Colors;

  module Styles = {
    open Style;

    let container = [
      position(`Absolute),
      top(0),
      left(0),
      bottom(0),
      right(0),
      padding(10),
    ];

    let commit = [flexDirection(`Row)];

    let groupHeader = (font: UiFont.t) => [
      fontFamily(font.fontFile),
      fontSize(16.),
      marginTop(4),
    ];

    let groupBody = [paddingLeft(10)];

    let typ = (font: UiFont.t, ~color) => [
      fontFamily(font.fontFile),
      fontSize(12.),
      Style.color(color),
      width(80),
    ];

    let scope = (font: UiFont.t, ~theme) => [
      fontFamily(font.fontFile),
      fontSize(12.),
      color(Revery.Colors.grey),
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
      | Some("perf") => "performance"
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
        <date commit={List.hd(commits)} style={Styles.groupHeader(uiFont)} />
        <View style=Styles.groupBody>
          {commits
           |> List.map(commit => <line commit uiFont theme />)
           |> React.listToElement}
        </View>
      </View>;
    };

    let make = (~theme, ~uiFont, ~editorFont as _, ()) => {
      let isSignificantCommit = commit =>
        switch (commit.typ) {
        | Some("feat" | "fix" | "perf") => true
        | _ => false
        };

      switch (read()) {
      | Ok(commits) =>
        <ScrollView style=Styles.container>
          {commits
           |> List.filter(isSignificantCommit)
           |> Base.List.group(~break=(a, b) => !isSameDate(a.time, b.time))
           |> List.map(commits => <group commits uiFont theme />)
           |> React.listToElement}
        </ScrollView>
      | Error(message) => <Text style={Styles.error(uiFont)} text=message />
      };
    };
  };
};
