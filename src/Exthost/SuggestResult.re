open Oni_Core;

[@deriving show]
type t = {
  completions: list(SuggestItem.t),
  isIncomplete: bool,
};

let empty = {completions: [], isIncomplete: false};

module Dto = {
  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          // These values come from `ISuggestResultDto`:
          // https://github.com/onivim/vscode-exthost/blob/50bef147f7bbd250015361a4e3cad3305f65bc27/src/vs/workbench/api/common/extHost.protocol.ts#L1129
          completions: field.required("b", list(SuggestItem.Dto.decode)),
          isIncomplete: field.withDefault("c", false, bool),
        }
      )
    );
  };
};
