/*
 * ExtensionContributions.re
 *
 * Types for VSCode Extension contribution points
 */
open Rench;

module Commands = {
  [@deriving (show, yojson({strict: false, exn: true}))]
  type t = {
    command: string,
    title: string,
    category: [@default None] option(string),
  };
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
    let path = Path.join(path, g.path);

    let treeSitterPath =
      switch (g.treeSitterPath) {
      | Some(v) => Some(Path.join(path, v))
      | None => None
      };

    {...g, path, treeSitterPath};
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

let remapPaths = (path: string, contributions: t) => {
  ...contributions,
  grammars: _remapGrammars(path, contributions.grammars),
  themes: _remapThemes(path, contributions.themes),
  languages: _remapLanguages(path, contributions.languages),
  iconThemes: _remapIconThemes(path, contributions.iconThemes),
};
