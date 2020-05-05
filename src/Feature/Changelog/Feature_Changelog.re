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

module View = {
  open Revery.UI;

  module Styles = {
    open Style;

    let text = (font: UiFont.t) => [fontFamily(font.fontFile)];
  };

  module Full = {
    let make = (~font, ()) => {
      switch (read()) {
      | Ok(commits) =>
        commits
        |> List.map(commit => {
             let text = Printf.sprintf("%s: %s", commit.hash, commit.summary);
             <Text style={Styles.text(font)} text />;
           })
        |> React.listToElement
      | Error(message) => <Text style={Styles.text(font)} text=message />
      };
    };
  };
};
