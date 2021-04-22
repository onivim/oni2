/*
 * LanguageConfiguration.re
 */

open Oniguruma;

open Utility;

module Log = (val Kernel.Log.withNamespace("Oni2.LanguageConfiguration"));

let json = Yojson.Safe.from_string;

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
    let%test "decode tuple" = {
      let acp = json({|["a", "b"]|}) |> decode_value(decode);

      acp == Ok({openPair: "a", closePair: "b", notIn: []});
    };
    let%test "decode simple object" = {
      let acp =
        json({|{"open": "c", "close": "d"}|}) |> decode_value(decode);

      acp == Ok({openPair: "c", closePair: "d", notIn: []});
    };
    let%test "decode object with notIn list" = {
      let acp =
        json({|{"open": "c", "close": "d", "notIn": ["string"]}|})
        |> decode_value(decode);

      acp == Ok({openPair: "c", closePair: "d", notIn: [String]});
    };
    let%test "decode object with notIn string" = {
      let acp =
        json({|{"open": "c", "close": "d", "notIn": "comment"}|})
        |> decode_value(decode);

      acp == Ok({openPair: "c", closePair: "d", notIn: [Comment]});
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
  wordPattern: option(OnigRegExp.t),
};

let brackets = ({brackets, _}) => brackets;
let lineComment = ({lineComment, _}) => lineComment;

let blockComment = ({blockComment, _}) => blockComment;

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
  wordPattern:
    OnigRegExp.create(
      "(-?\\d*\\.\\d\\w*)|([^\\`\\~\\!\\@\\#\\%\\^\\&\\*\\(\\)\\-\\=\\+\\[\\{\\]\\}\\\\\\|\\;\\:\\'\\\"\\,\\.\\<\\>\\/\\?\\s]+)",
    )
    |> Result.to_option,
};

module CustomDecoders = {
  open Json.Decode;
  let autoCloseBeforeDecode = string |> map(StringEx.explode);
};

module Decode = {
  let _defaultConfig = default;
  open Json.Decode;
  open CustomDecoders;

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
            regexp |> map(Option.some),
          ),
        decreaseIndentPattern:
          at.withDefault(
            ["indentationRules", "decreaseIndentPattern"],
            None,
            regexp |> map(Option.some),
          ),

        wordPattern:
          field.withDefault(
            "wordPattern",
            _defaultConfig.wordPattern,
            regexp |> map(Option.some),
          ),
      }
    );

  let%test "decode autoCloseBefore" = {
    let languageConfig =
      json({|{"autoCloseBefore": "abc"}|})
      |> decode_value(configuration)
      |> Result.get_ok;

    languageConfig.autoCloseBefore == ["a", "b", "c"];
  };
  let%test "decode brackets" = {
    let languageConfig =
      json({|{"brackets": [["{", "}"]]}|})
      |> decode_value(configuration)
      |> Result.get_ok;

    languageConfig.brackets == [{openPair: "{", closePair: "}"}];
  };
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

let shouldIncreaseIndent =
    (
      ~previousLine as str,
      ~beforePreviousLine as _,
      {increaseIndentPattern, brackets, _},
    ) => {
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
};

let shouldDecreaseIndent = (~line, {decreaseIndentPattern, brackets, _}) => {
  decreaseIndentPattern
  |> Option.map(regex => OnigRegExp.test(line, regex))
  |> OptionEx.or_lazy(() => {
       Some(
         List.exists(
           bracket => BracketPair.isJustClosingPair(bracket, line),
           brackets,
         ),
       )
     })
  |> Option.value(~default=false);
};

let toAutoIndent = (languageConfig, ~previousLine, ~beforePreviousLine) => {
  let increase =
    shouldIncreaseIndent(~previousLine, ~beforePreviousLine, languageConfig);

  if (increase) {Vim.AutoIndent.IncreaseIndent} else {
    Vim.AutoIndent.KeepIndent
  };
};

let isWordCharacter = (char, {wordPattern, _}) => {
  wordPattern
  |> Option.map(regexp =>
       OnigRegExp.Fast.test(Zed_utf8.make(1, char), regexp)
     )
  |> Option.value(~default=false);
};

let%test_module "LanguageConfiguration" =
  (module
   {
     let defaultConfig = default;
     open Json.Decode;

     let%test "increase / decrease indent" = {
       let languageConfig =
         json(
           {|
        {"indentationRules":
          {
          "increaseIndentPattern":"abc",
          "decreaseIndentPattern":"def"
          }
        }|},
         )
         |> decode_value(decode)
         |> Result.get_ok;

       toAutoIndent(
         languageConfig,
         ~previousLine="abc",
         ~beforePreviousLine=None,
       )
       == Vim.AutoIndent.IncreaseIndent;
     };
     let%test "falls back to brackets" = {
       let languageConfig =
         json({|
        {"brackets":
		  [["{", "}"]]
        }|})
         |> decode_value(decode)
         |> Result.get_ok;

       toAutoIndent(
         languageConfig,
         ~previousLine="   {",
         ~beforePreviousLine=None,
       )
       == Vim.AutoIndent.IncreaseIndent;
     };
     let%test "default word pattern: isWordCharacter true for 'a'" = {
       isWordCharacter(Uchar.of_char('a'), defaultConfig) == true;
     };
     let%test "default word pattern: isWordCharacter false for ' '" = {
       isWordCharacter(Uchar.of_char(' '), defaultConfig) == false;
     };
     let%test "default word pattern: isWordCharacter false for '('" = {
       isWordCharacter(Uchar.of_char('('), defaultConfig) == false;
     };
   });
