open Oni_Core;

module QuickSuggestionsSetting = {
  [@deriving show]
  type t = {
    comments: bool,
    strings: bool,
    other: bool,
  };

  let initial = {comments: false, strings: false, other: true};

  let enabledFor = (~syntaxScope: SyntaxScope.t, {comments, strings, other}) => {
    let isCommentAndAllowed = syntaxScope.isComment && comments;

    let isStringAndAllowed = syntaxScope.isString && strings;

    let isOtherAndAllowed =
      !syntaxScope.isComment && !syntaxScope.isString && other;

    isCommentAndAllowed || isStringAndAllowed || isOtherAndAllowed;
  };

  module Decode = {
    open Json.Decode;
    let decodeBool =
      bool
      |> map(
           fun
           | false => {comments: false, strings: false, other: false}
           | true => {comments: true, strings: true, other: true},
         );

    let decodeObj =
      obj(({field, _}) =>
        {
          comments: field.withDefault("comments", initial.comments, bool),
          strings: field.withDefault("strings", initial.strings, bool),
          other: field.withDefault("other", initial.other, bool),
        }
      );

    let decode = one_of([("bool", decodeBool), ("obj", decodeObj)]);
  };

  let decode = Decode.decode;

  let encode = setting =>
    Json.Encode.(
      {
        obj([
          ("comments", setting.comments |> bool),
          ("strings", setting.strings |> bool),
          ("other", setting.other |> bool),
        ]);
      }
    );
};

module Decode = {
  open Json.Decode;

  module AcceptSuggestionOnEnter = {
    let decodeBool = bool;
    let decodeString =
      string
      |> map(
           fun
           | "on" => true
           // TODO: "smart" setting?
           | _ => false,
         );

    let decode =
      one_of([
        ("acceptSuggestionOnEnter.bool", decodeBool),
        ("acceptSuggestionOnEnter.string", decodeString),
      ]);
  };
};

// CONFIGURATION

open Config.Schema;

let quickSuggestions =
  setting(
    "editor.quickSuggestions",
    custom(
      ~decode=QuickSuggestionsSetting.decode,
      ~encode=QuickSuggestionsSetting.encode,
    ),
    ~default=QuickSuggestionsSetting.initial,
  );

let wordBasedSuggestions =
  setting("editor.wordBasedSuggestions", bool, ~default=true);

let acceptSuggestionOnEnter =
  setting(
    "editor.acceptSuggestionOnEnter",
    custom(
      ~decode=Decode.AcceptSuggestionOnEnter.decode,
      ~encode=Json.Encode.bool,
    ),
    ~default=true,
  );
