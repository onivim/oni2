/*
 * ExtensionContributions.re
 *
 * Types for VSCode Extension contribution points
 */

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

[@deriving (show, yojson({strict: false, exn: true}))]
type t = {
  commands: [@default []] list(Commands.t),
  languages: [@default []] list(Language.t),
  grammars: [@default []] list(Grammar.t),
  themes: [@default []] list(Theme.t),
};
