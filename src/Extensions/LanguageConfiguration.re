/*
 * LanguageConfiguration.re
 */

open Oni_Core.Utility;
module Json = Oni_Core.Json;
module SyntaxScope = Oni_Core.SyntaxScope;

module AutoClosingPair = {
  type scopes =
    | String
    | Comment
    | Other(string);

  type t = {
    openPair: string,
    closePair: string,
    notIn: list(scopes),
  };

  let isActive = ({isComment, isString}: SyntaxScope.t, acp: t) => {
    let disallowString = acp.notIn |> List.exists(notIn => notIn == String);
    let disallowComment = acp.notIn |> List.exists(notIn => notIn == Comment);

    (!isComment || !disallowComment) && (!isString || !disallowString);
  };

  module Decode = {
    open Json.Decode;

    let scope =
      string
      |> map(
           fun
           | "string" => String
           | "comment" => Comment
           | v => Other(v),
         );

    let scopeToList = scope |> map(v => [v]);

    let tuple =
      list(string)
      |> and_then(
           fun
           | [openPair, closePair] =>
             succeed({openPair, closePair, notIn: []})
           | _ => fail("Expected a 2-element tuple"),
         );

    let obj =
      obj(({field, whatever, _}) =>
        {
          openPair: field.required("open", string),
          closePair: field.required("close", string),
          notIn:
            whatever(
              one_of([
                ("scopeList", field.monadic("notIn", list(scope))),
                ("scopeString", field.monadic("notIn", scopeToList)),
                ("default", succeed([])),
              ]),
            ),
        }
      );

    let decode = {
      one_of([("tuple", tuple), ("object", obj)]);
    };
  };

  let decode = Decode.decode;
};

type t = {
  autoCloseBefore: list(string),
  autoClosingPairs: list(AutoClosingPair.t),
};

let default = {
  autoCloseBefore: [
    ";",
    ":",
    ".",
    ",",
    "=",
    "}",
    "]",
    ")",
    ">",
    "`",
    " ",
    "\t",
  ],
  autoClosingPairs: [],
};

module Decode = {
  let _defaultConfig = default;
  open Json.Decode;

  let autoCloseBeforeDecode = string |> map(StringEx.explode);

  let configuration =
    obj(({field, _}) =>
      {
        autoCloseBefore:
          field.withDefault(
            "autoCloseBefore",
            _defaultConfig.autoCloseBefore,
            autoCloseBeforeDecode,
          ),
        autoClosingPairs:
          field.withDefault(
            "autoClosingPairs",
            [],
            list(AutoClosingPair.decode),
          ),
      }
    );
};

let decode = Decode.configuration;

let toVimAutoClosingPairs = (syntaxScope: SyntaxScope.t, configuration: t) => {
  let toAutoPair = ({openPair, closePair, _}: AutoClosingPair.t) => {
    Vim.AutoClosingPairs.AutoPair.{opening: openPair, closing: closePair};
  };

  let pairs =
    configuration.autoClosingPairs
    |> List.filter(AutoClosingPair.isActive(syntaxScope))
    |> List.map(toAutoPair);

  let passThrough =
    configuration.autoClosingPairs
    |> List.map(({closePair, _}: AutoClosingPair.t) => closePair);

  let deletionPairs = configuration.autoClosingPairs |> List.map(toAutoPair);

  Vim.AutoClosingPairs.create(
    ~passThrough,
    ~deletionPairs,
    ~allowBefore=configuration.autoCloseBefore,
    pairs,
  );
};
