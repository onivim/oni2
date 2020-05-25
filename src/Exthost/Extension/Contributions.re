/*
 * Contributions.re
 *
 * Types for VSCode Extension contribution points
 */

open Oni_Core;
open Rench;

module Command = {
  [@deriving show]
  type t = {
    command: string,
    title: LocalizedToken.t,
    category: option(string),
    condition: WhenExpr.t,
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          command: field.required("command", string),
          title: field.required("title", LocalizedToken.decode),
          category: field.optional("category", string),
          condition:
            field.withDefault(
              "when",
              WhenExpr.Value(True),
              string |> map(WhenExpr.parse),
            ),
        }
      )
    );

  let encode = command =>
    Json.Encode.(
      obj([
        ("command", command.command |> string),
        ("title", null),
        ("category", command.category |> nullable(string)),
      ])
    );
};

module Menu = {
  [@deriving show]
  type t = Menu.Schema.definition

  and item = Menu.Schema.item;

  module Decode = {
    open Json.Decode;

    let whenExpression = string |> map(WhenExpr.parse);

    let item =
      obj(({field, _}) =>
        Menu.Schema.{
          command: field.required("command", string),
          alt: field.optional("alt", string),
          group: field.optional("group", string),
          index: None,
          isVisibleWhen:
            field.withDefault(
              "when",
              WhenExpr.Value(True),
              string |> map(WhenExpr.parse),
            ),
        }
      );

    let menus =
      key_value_pairs(list(item))
      |> map(List.map(((id, items)) => Menu.Schema.{id, items}));
  };
};

module Configuration = {
  [@deriving show]
  type t = list(property)

  and property = {
    name: string,
    default: [@opaque] Json.t,
    // TODO:
    // type
    // description
    // scope
  };

  module Decode = {
    open Json.Decode;

    let property = name =>
      field_opt("default", value)
      |> default(Json.Encode.null)
      |> map(default => {name, default});

    let properties = key_value_pairs_seq(property);

    let configuration = {
      let simple = field("properties", properties);

      one_of([
        ("single", simple),
        ("list", list(simple) |> map(List.flatten)),
      ]);
    };
  };

  let decode = Decode.configuration;

  let toSettings = config =>
    config
    |> List.map(({name, default}) => (name, default))
    |> Config.Settings.fromList;
};

module Language = {
  [@deriving show]
  type t = {
    id: string,
    extensions: list(string),
    aliases: list(string),
    configuration: option(string),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.required("id", string),
          extensions: field.withDefault("extensions", [], list(string)),
          aliases: field.withDefault("aliases", [], list(string)),
          configuration: field.optional("configuration", string),
        }
      )
    );

  let encode = language =>
    Json.Encode.(
      obj([
        ("id", language.id |> string),
        ("extensions", language.extensions |> list(string)),
        ("aliases", language.aliases |> list(string)),
        ("configuration", language.configuration |> nullable(string)),
      ])
    );
};

module Grammar = {
  [@deriving show]
  type t = {
    language: option(string),
    scopeName: string,
    path: string,
    treeSitterPath: option(string),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          language: field.optional("language", string),
          scopeName: field.required("scopeName", string),
          path: field.required("path", string),
          treeSitterPath: field.optional("treeSitterPath", string),
        }
      )
    );

  let encode = grammar =>
    Json.Encode.(
      obj([
        ("language", grammar.language |> nullable(string)),
        ("scopeName", grammar.scopeName |> string),
        ("path", grammar.path |> string),
        ("treeSitterPath", grammar.treeSitterPath |> nullable(string)),
      ])
    );

  let toAbsolutePath = (base: string, grammar) => {
    let path = Path.join(base, grammar.path);

    let treeSitterPath =
      grammar.treeSitterPath |> Option.map(Path.join(base));

    {...grammar, path, treeSitterPath};
  };
};

module Theme = {
  [@deriving show]
  type t = {
    label: string,
    uiTheme: string,
    path: string,
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          label: field.required("label", string),
          uiTheme: field.required("uiTheme", string),
          path: field.required("path", string),
        }
      )
    );

  let encode = theme =>
    Json.Encode.(
      obj([
        ("label", theme.label |> string),
        ("uiTheme", theme.uiTheme |> string),
        ("path", theme.path |> string),
      ])
    );
};

module IconTheme = {
  [@deriving show]
  type t = {
    id: string,
    label: string,
    path: string,
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.required("id", string),
          label: field.required("label", string),
          path: field.required("path", string),
        }
      )
    );

  let encode = theme =>
    Json.Encode.(
      obj([
        ("id", theme.id |> string),
        ("label", theme.label |> string),
        ("path", theme.path |> string),
      ])
    );
};

[@deriving show]
type t = {
  commands: list(Command.t),
  menus: list(Menu.t),
  languages: list(Language.t),
  grammars: list(Grammar.t),
  themes: list(Theme.t),
  iconThemes: list(IconTheme.t),
  configuration: Configuration.t,
};

let default = {
  commands: [],
  menus: [],
  languages: [],
  grammars: [],
  themes: [],
  iconThemes: [],
  configuration: [],
};

let decode =
  Json.Decode.(
    obj(({field, _}) =>
      {
        commands: field.withDefault("commands", [], list(Command.decode)),
        languages: field.withDefault("languages", [], list(Language.decode)),
        grammars: field.withDefault("grammars", [], list(Grammar.decode)),
        themes: field.withDefault("themes", [], list(Theme.decode)),
        menus: field.withDefault("menus", [], Menu.Decode.menus),
        iconThemes:
          field.withDefault("iconThemes", [], list(IconTheme.decode)),
        configuration:
          field.withDefault("configuration", [], Configuration.decode),
      }
    )
  );

let encode = data =>
  Json.Encode.(
    obj([
      ("commands", data.commands |> list(Command.encode)),
      ("menus", null),
      ("languages", data.languages |> list(Language.encode)),
      ("grammars", data.grammars |> list(Grammar.encode)),
      ("themes", data.themes |> list(Theme.encode)),
      ("iconThemes", data.iconThemes |> list(IconTheme.encode)),
      ("configuration", null),
    ])
  );

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
       Command.{...cmd, title: LocalizedToken.localize(loc, cmd.title)}
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
