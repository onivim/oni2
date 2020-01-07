/*
 * ExtensionContributions.re
 *
 * Types for VSCode Extension contribution points
 */
open Oni_Core;
open Rench;

module Commands = {
  [@deriving (show, yojson({strict: false, exn: true}))]
  type t = {
    command: string,
    title: LocalizedToken.t,
    category: [@default None] option(string),
  };
};

module Configuration = {
  [@deriving show]
  type config = {
    name: string,
    [@opaque]
    default: Yojson.Safe.json,
    // TODO:
    // type
    // description
    // scope
  };

  let config_of_yojson_exn = (~name, json) => {
    Yojson.Safe.(
      {
        let default = Util.member("default", json);
        {name, default};
      }
    );
  };

  [@deriving show]
  type t = list(config);

  let of_yojson_exn = json => {
    Yojson.Safe.(
      {
        let parseConfigSection = json => {
          let properties = Util.member("properties", json) |> Util.to_assoc;
          properties
          |> List.map(prop => {
               let (key, configJson) = prop;
               config_of_yojson_exn(~name=key, configJson);
             });
        };

        switch (json) {
        | `List(items) => List.map(parseConfigSection, items) |> List.flatten
        | json => parseConfigSection(json)
        };
      }
    );
  };

  let of_yojson = json =>
    Oni_Core.Utility.tryToResult(~msg="Error parsing", () =>
      of_yojson_exn(json)
    );

  let to_yojson = _v => `Null;
};

module Language = {
  [@deriving (show, yojson({strict: false, exn: true}))]
  type t = {
    id: string,
    extensions: [@default []] list(string),
    aliases: [@default []] list(string),
    configuration: [@default None] option(string),
  };
};

module Grammar = {
  [@deriving (show, yojson({strict: false, exn: true}))]
  type t = {
    language: [@default None] option(string),
    scopeName: string,
    path: string,
    treeSitterPath: [@default None] option(string),
  };

  let toAbsolutePath = (path: string, g: t) => {
    let grammarPath = Path.join(path, g.path);

    let treeSitterPath =
      switch (g.treeSitterPath) {
      | Some(v) => Some(Path.join(path, v))
      | None => None
      };

    {...g, path: grammarPath, treeSitterPath};
  };
};

module Theme = {
  [@deriving (show, yojson({strict: false, exn: true}))]
  type t = {
    label: string,
    uiTheme: string,
    path: string,
  };
};

module IconTheme = {
  [@deriving (show, yojson({strict: false, exn: true}))]
  type t = {
    id: string,
    label: string,
    path: string,
  };
};

[@deriving (show, yojson({strict: false, exn: true}))]
type t = {
  commands: [@default []] list(Commands.t),
  languages: [@default []] list(Language.t),
  grammars: [@default []] list(Grammar.t),
  themes: [@default []] list(Theme.t),
  iconThemes: [@default []] list(IconTheme.t),
  [@default None]
  configuration: option(Configuration.t),
};

let _remapGrammars = (path: string, grammars: list(Grammar.t)) => {
  List.map(g => Grammar.toAbsolutePath(path, g), grammars);
};

let _remapThemes = (path: string, themes: list(Theme.t)) => {
  List.map(t => Theme.{...t, path: Path.join(path, t.path)}, themes);
};

let _remapLanguages = (path: string, languages: list(Language.t)) => {
  let remapPath = p =>
    switch (p) {
    | None => None
    | Some(v) => Some(Path.join(path, v))
    };

  List.map(
    l => Language.{...l, configuration: remapPath(l.configuration)},
    languages,
  );
};

let _remapIconThemes = (path: string, themes: list(IconTheme.t)) => {
  List.map(t => IconTheme.{...t, path: Path.join(path, t.path)}, themes);
};

let _localizeCommands = (loc, cmds) => {
  cmds
  |> List.map(cmd =>
       Commands.{...cmd, title: LocalizedToken.localize(loc, cmd.title)}
     );
};

let remapPaths = (path: string, contributions: t) => {
  ...contributions,
  grammars: _remapGrammars(path, contributions.grammars),
  themes: _remapThemes(path, contributions.themes),
  languages: _remapLanguages(path, contributions.languages),
  iconThemes: _remapIconThemes(path, contributions.iconThemes),
};

let localize = (locDictionary: LocalizationDictionary.t, contributions: t) => {
  ...contributions,
  commands: _localizeCommands(locDictionary, contributions.commands),
};

let getConfiguration = (manifest: t) => {
  Utility.Option.value(~default=[], manifest.configuration);
};
