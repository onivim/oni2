// LanguageConfiguration.re models the extension-provided language configuration:
// https://github.com/onivim/vscode-exthost/blob/0d6b39803352369daaa97a444ff76352d8452be2/src/vs/workbench/api/common/extHost.protocol.ts#L341

open Oniguruma;
open Oni_Core;

module IndentAction = {
  [@deriving show]
  type t =
    | Indent
    | IndentOutdent
    | Outdent;

  // Must be in sync with: https://github.com/microsoft/vscode/blob/39ea9cac052a2e5e05f7d6c0911078415f506e29/src/vs/editor/common/modes/languageConfiguration.ts#L192
  let ofInt =
    fun
    | 1 => Some(Indent)
    | 2 => Some(IndentOutdent)

    | 3 => Some(Outdent)
    | _ => None;

  let decode =
    Json.Decode.(
      int
      |> map(ofInt)
      |> and_then(
           fun
           | None => fail("Unable to parse IndentAction")
           | Some(action) => succeed(action),
         )
    );
};

module EnterAction = {
  [@deriving show]
  type t = {
    indentAction: IndentAction.t,
    appendText: option(string),
    removeText: option(int),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          indentAction: field.required("indentAction", IndentAction.decode),
          appendText: field.optional("appendText", string),
          removeText: field.optional("removeText", int),
        }
      )
    );
};

module OnEnterRule = {
  [@deriving show]
  type t = {
    beforeText: [@opaque] OnigRegExp.t,
    afterText: [@opaque] option(OnigRegExp.t),
    previousLineText: [@opaque] option(OnigRegExp.t),
    action: EnterAction.t,
  };

  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          beforeText: field.required("beforeText", regexp),
          afterText: field.optional("afterText", regexp),
          previousLineText: field.optional("previousLineText", regexp),
          action: field.required("action", EnterAction.decode),
        }
      )
    );
  };
};

[@deriving show]
type t = {onEnterRules: list(OnEnterRule.t)};

let decode =
  Json.Decode.(
    obj(({field, _}) =>
      {
        onEnterRules:
          field.withDefault("onEnterRules", [], list(OnEnterRule.decode)),
      }
    )
  );
