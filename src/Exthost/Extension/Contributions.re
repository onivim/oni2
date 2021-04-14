/*
 * Contributions.re
 *
 * Types for VSCode Extension contribution points
 */

open Oni_Core;
open Rench;

module Log = (val Log.withNamespace("Exthost.Extension.Contributions"));

module Breakpoint = {
  [@deriving show]
  type t = Yojson.Safe.t;
  let decode = Json.Decode.value;
  let encode = Json.Encode.value;
};

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
              one_of([
                ("when.string", string |> map(WhenExpr.parse)),
                (
                  "when.bool",
                  bool
                  |> map(
                       fun
                       | true => WhenExpr.Value(True)
                       | false => WhenExpr.Value(False),
                     ),
                ),
              ]),
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

module Debugger = {
  [@deriving show]
  type t = Yojson.Safe.t;

  let decode = Json.Decode.value;
  let encode = Json.Encode.value;
};

module Menu = {
  [@deriving show]
  type t = Menu.Schema.definition

  and item = Menu.Schema.item;

  module Decode = {
    open Json.Decode;

    let whenExpression = string |> map(WhenExpr.parse);

    let command =
      obj(({field, _}) =>
        Menu.Schema.(
          Command({
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
          })
        )
      );

    let submenu =
      obj(({field, _}) =>
        Menu.Schema.(
          Submenu({
            submenu: field.required("submenu", string),
            group: field.optional("group", string),
            isVisibleWhen:
              field.withDefault(
                "when",
                WhenExpr.Value(True),
                string |> map(WhenExpr.parse),
              ),
          })
        )
      );

    let item = one_of([("command", command), ("submenu", submenu)]);

    let menus =
      key_value_pairs(list(item))
      |> map(List.map(((id, items)) => Menu.Schema.{id, items}));
  };
};

module Configuration = {
  module PropertyType = {
    [@deriving show]
    type t =
      | Array
      | Boolean
      | String
      | Integer
      | Number
      | Object
      | Unknown;

    let default: t => Yojson.Safe.t =
      fun
      | Array => `List([])
      | String => `String("")
      | Integer => `Int(0)
      | Number => `Int(0)
      | Boolean => `Bool(false)
      | Object => `Assoc([])
      | Unknown => `Null;

    module Decode = {
      open Json.Decode;

      let list = list(nullable(string)) |> map(_ => Unknown);

      let single =
        string
        |> map(String.lowercase_ascii)
        |> and_then(
             fun
             | "array" => succeed(Array)
             | "boolean" => succeed(Boolean)
             | "string" => succeed(String)
             | "integer" => succeed(Integer)
             | "number" => succeed(Number)
             | "object" => succeed(Object)
             | unknown => {
                 Log.warnf(m => m("Unknown configuration type: %s", unknown));
                 succeed(Unknown);
               },
           );

      let decode = one_of([("single", single), ("list", list)]);
    };

    let decode = Decode.decode;
  };

  [@deriving show]
  type t = list(property)
  and property = {
    name: string,
    default: [@opaque] Json.t,
    propertyType: PropertyType.t,
    // TODO:
    // description
    // scope
    // enum
  };

  module Decode = {
    open Json.Decode;

    let setDefaultIfNecessary = prop => {
      switch (prop.default) {
      | `Null => {...prop, default: PropertyType.default(prop.propertyType)}
      | _ => prop
      };
    };

    let property = name =>
      obj(({field, _}) => {
        let propertyType =
          field.withDefault(
            "type",
            PropertyType.Unknown,
            PropertyType.decode,
          );

        let default = PropertyType.default(propertyType);
        {
          name,
          default: field.withDefault("default", default, value),
          propertyType,
        };
      })
      |> map(setDefaultIfNecessary);

    let properties = key_value_pairs_seq(property);

    let configuration = {
      let simple = field("properties", properties);

      one_of([
        ("single", simple),
        ("list", list(simple) |> map(List.flatten)),
      ]);
    };

    let%test_module "decode" =
      (module
       {
         let ofString = (decoder, str) =>
           str
           |> Yojson.Safe.from_string
           |> Json.Decode.decode_value(decoder)
           |> Result.get_ok;

         let expectEquals = (a, b) => {
           a == b;
         };

         let%test "property: bool, no type" = {
           {|
            {
              "default": false
            }
          |}
           |> ofString(property("boolprop"))
           |> expectEquals({
                name: "boolprop",
                // TODO:
                // The property could/should be inferred by the default value
                propertyType: Unknown,
                default: `Bool(false),
              });
         };

         let%test "property: string, no type" = {
           {|
            {
              "default": "set",
            }
          |}
           |> ofString(property("stringprop"))
           |> expectEquals({
                name: "stringprop",
                // TODO:
                // The property could/should be inferred by the default value
                propertyType: Unknown,
                default: `String("set"),
              });
         };

         let%test "property: boolean, default" = {
           {|
            {
              "type": "boolean",
              "default": false
            }
          |}
           |> ofString(property("boolprop"))
           |> expectEquals({
                name: "boolprop",
                propertyType: Boolean,
                default: `Bool(false),
              });
         };

         let%test "property: boolean, no default" = {
           {|
            {
              "type": "boolean",
            }
          |}
           |> ofString(property("boolprop"))
           |> expectEquals({
                name: "boolprop",
                propertyType: Boolean,
                default: `Bool(false),
              });
         };

         let%test "property: number, no default" = {
           {|
            {
              "type": "number",
            }
          |}
           |> ofString(property("prop"))
           |> expectEquals({
                name: "prop",
                propertyType: Number,
                default: `Int(0),
              });
         };

         let%test "property: number, with default" = {
           {|
            {
              "type": "number",
              "default": 111
            }
          |}
           |> ofString(property("prop"))
           |> expectEquals({
                name: "prop",
                propertyType: Number,
                default: `Int(111),
              });
         };

         let%test "property: number, no default" = {
           {|
            {
              "type": "object",
            }
          |}
           |> ofString(property("prop"))
           |> expectEquals({
                name: "prop",
                propertyType: Object,
                default: `Assoc([]),
              });
         };

         let%test "property: array, no default" = {
           {|
            {
              "type": "array",
            }
          |}
           |> ofString(property("arrayprop"))
           |> expectEquals({
                name: "arrayprop",
                propertyType: Array,
                default: `List([]),
              });
         };

         let%test "property: list of types, default already set" =
           {
             {|
        {
          "type": [
            "string",
            "null"
          ],
          "default": null,
          "markdownDescription": "%typescript.tsdk.desc%",
          "scope": "window"
        }
             |};
           }
           |> ofString(property("multiprop"))
           |> expectEquals({
                name: "multiprop",
                propertyType: Unknown,
                default: `Null,
              });
       });
  };

  let decode = Decode.configuration;

  let toSettings = config =>
    config
    |> List.map(({name, default, _}) => (name, default))
    |> Config.Settings.fromList;
};

module Language = {
  [@deriving show]
  type t = {
    id: string,
    extensions: list(string),
    filenames: list(string),
    filenamePatterns: list(string),
    firstLine: option(string),
    aliases: list(string),
    configuration: option(string),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.required("id", string),
          extensions: field.withDefault("extensions", [], list(string)),
          filenames: field.withDefault("filenames", [], list(string)),
          filenamePatterns:
            field.withDefault("filenamePatterns", [], list(string)),
          firstLine: field.optional("firstLine", string),
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
        ("filenames", language.filenames |> list(string)),
        ("filenamePatterns", language.filenamePatterns |> list(string)),
        ("firstLine", language.firstLine |> nullable(string)),
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

module Snippet = {
  [@deriving show]
  type t = {
    language: option(string),
    path: string,
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          language: field.optional("language", string),
          path: field.required("path", string),
        }
      )
    );

  let encode = snippet =>
    Json.Encode.(
      obj([
        ("language", snippet.language |> nullable(string)),
        ("path", snippet.path |> string),
      ])
    );

  let toAbsolutePath = (base: string, snippet) => {
    let path = Path.join(base, snippet.path);

    {...snippet, path};
  };
};

module Theme = {
  [@deriving show]
  type t = {
    id: option(string),
    label: LocalizedToken.t,
    uiTheme: string,
    path: string,
  };

  let localize = (loc: LocalizationDictionary.t, theme) => {
    ...theme,
    label: LocalizedToken.localize(loc, theme.label),
  };

  let id = ({id, label, _}) =>
    id |> Option.value(~default=LocalizedToken.toString(label));

  let label = ({label, _}) => LocalizedToken.toString(label);

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.optional("id", string),
          label: field.required("label", LocalizedToken.decode),
          uiTheme: field.required("uiTheme", string),
          path: field.required("path", string),
        }
      )
    );

  let encode = theme =>
    Json.Encode.(
      obj([
        ("id", theme.id |> nullable(string)),
        ("label", theme.label |> LocalizedToken.encode),
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
  breakpoints: list(Breakpoint.t),
  commands: list(Command.t),
  debuggers: list(Debugger.t),
  menus: list(Menu.t),
  languages: list(Language.t),
  grammars: list(Grammar.t),
  snippets: list(Snippet.t),
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
  debuggers: [],
  snippets: [],
  breakpoints: [],
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
        debuggers: field.withDefault("debuggers", [], list(Debugger.decode)),
        breakpoints:
          field.withDefault("breakpoints", [], list(Breakpoint.decode)),
        snippets: field.withDefault("snippets", [], list(Snippet.decode)),
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
      ("debuggers", data.debuggers |> list(Debugger.encode)),
      ("breakpoints", data.breakpoints |> list(Breakpoint.encode)),
      ("snippets", data.snippets |> list(Snippet.encode)),
    ])
  );
let to_yojson = contributes => {
  contributes |> Json.Encode.encode_value(encode);
};

let of_yojson = json => {
  json
  |> Json.Decode.decode_value(decode)
  |> Result.map_error(Json.Decode.string_of_error);
};

let _remapGrammars = (path: string, grammars: list(Grammar.t)) => {
  List.map(g => Grammar.toAbsolutePath(path, g), grammars);
};

let _remapSnippets = (path: string, snippets: list(Snippet.t)) => {
  List.map(g => Snippet.toAbsolutePath(path, g), snippets);
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

let _localizeThemes = (loc, themes) => {
  themes |> List.map(theme => Theme.localize(loc, theme));
};

let remapPaths = (path: string, contributions: t) => {
  ...contributions,
  grammars: _remapGrammars(path, contributions.grammars),
  themes: _remapThemes(path, contributions.themes),
  languages: _remapLanguages(path, contributions.languages),
  iconThemes: _remapIconThemes(path, contributions.iconThemes),
  snippets: _remapSnippets(path, contributions.snippets),
};

let localize = (locDictionary: LocalizationDictionary.t, contributions: t) => {
  ...contributions,
  commands: _localizeCommands(locDictionary, contributions.commands),
  themes: _localizeThemes(locDictionary, contributions.themes),
};
