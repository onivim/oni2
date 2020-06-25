/*
 * LanguageConfiguration.re
 */

open Oniguruma;

open Utility;

module Log = (val Kernel.Log.withNamespace("Oni2.LanguageConfiguration"));

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

module BracketPair = {
  type t = {
    openPair: string,
    closePair: string,
  };

  let decode =
    Json.Decode.(
      Pipeline.(
        decode((openPair, closePair) => {openPair, closePair})
        |> custom(index(0, string))
        |> custom(index(1, string))
      )
    );

  let endsWithOpenPair = ({openPair, _}, str) => {
    StringEx.endsWith(~postfix=openPair, str);
  };

  let isJustClosingPair = ({closePair, _}, str) => {
    let len = String.length(str);

    let rec loop = (foundPair, idx) =>
      if (idx >= len) {
        foundPair;
      } else if (foundPair) {
        false;
             // We found the closing pair... but there's other stuff after
      } else {
        let c = str.[idx];

        if (c == ' ' || c == '\t') {
          loop(foundPair, idx + 1);
        } else if (c == closePair.[0]) {
          loop(true, idx + 1);
        } else {
          false;
        };
      };

    if (String.length(closePair) == 1) {
      loop(false, 0);
    } else {
      false;
    };
  };
};

let defaultBrackets: list(BracketPair.t) =
  BracketPair.[
    {openPair: "{", closePair: "}"},
    {openPair: "[", closePair: "]"},
    {openPair: "(", closePair: ")"},
  ];

type t = {
  autoCloseBefore: list(string),
  autoClosingPairs: list(AutoClosingPair.t),
  brackets: list(BracketPair.t),
  lineComment: option(string),
  blockComment: option((string, string)),
  increaseIndentPattern: option(OnigRegExp.t),
  decreaseIndentPattern: option(OnigRegExp.t),
};

let default: t = {
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
  brackets: defaultBrackets,
  lineComment: None,
  blockComment: None,
  increaseIndentPattern: None,
  decreaseIndentPattern: None,
};

module Decode = {
  let _defaultConfig = default;
  open Json.Decode;

  let autoCloseBeforeDecode = string |> map(StringEx.explode);

  let regexp =
    string
    |> map(OnigRegExp.create)
    |> and_then(
         fun
         | Ok(regexp) => succeed(Some(regexp))
         | Error(msg) => {
             Log.errorf(m => m("Error %s parsing regex", msg));
             succeed(None);
           },
       );

  let configuration =
    obj(({field, at, _}) =>
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
        brackets:
          field.withDefault(
            "brackets",
            defaultBrackets,
            list(BracketPair.decode),
          ),
        lineComment: at.optional(["comments", "lineComment"], string),
        blockComment:
          at.optional(
            ["comments", "blockComment"],
            list(string)
            |> and_then(
                 fun
                 | [start, stop] => succeed((start, stop))
                 | _ => fail("Expected pair"),
               ),
          ),
        increaseIndentPattern:
          at.withDefault(
            ["indentationRules", "increaseIndentPattern"],
            None,
            regexp,
          ),
        decreaseIndentPattern:
          at.withDefault(
            ["indentationRules", "decreaseIndentPattern"],
            None,
            regexp,
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

let toOpenAutoIndent =
    (
      {increaseIndentPattern, brackets, _},
      ~previousLine as str,
      ~beforePreviousLine as _,
    ) => {
  let increase =
    increaseIndentPattern
    |> Option.map(regex => OnigRegExp.test(str, regex))
    // If no indentation pattern, fall-back to bracket pair
    |> OptionEx.or_lazy(() =>
         Some(
           List.exists(
             bracket => BracketPair.endsWithOpenPair(bracket, str),
             brackets,
           ),
         )
       )
    |> Option.value(~default=false);

  if (increase) {Vim.AutoIndent.IncreaseIndent} else {
    Vim.AutoIndent.KeepIndent
  };
};

let toTypeAutoIndent = ({decreaseIndentPattern, brackets, _}, str) => {
  let decrease =
    decreaseIndentPattern
    |> Option.map(regex => OnigRegExp.test(str, regex))
    // If no indentation pattern, fall-back to bracket pair
    |> OptionEx.or_lazy(() => {
         Some(
           List.exists(
             bracket => BracketPair.isJustClosingPair(bracket, str),
             brackets,
           ),
         )
       })
    |> Option.value(~default=false);

  if (decrease) {Vim.AutoIndent.DecreaseIndent} else {
    Vim.AutoIndent.KeepIndent
  };
};
