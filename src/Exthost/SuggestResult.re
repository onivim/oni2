open Oni_Core;

[@deriving show]
type cacheId = int;

[@deriving show]
type t = {
  completions: list(SuggestItem.t),
  isIncomplete: bool,
  cacheId: option(cacheId),
};

let empty = {completions: [], isIncomplete: false, cacheId: None};

module Dto = {
  let decode = {
    Json.Decode.(
      obj(({field, _}) => {
        // These values come from `ISuggestResultDto`:
        // https://github.com/onivim/vscode-exthost/blob/50bef147f7bbd250015361a4e3cad3305f65bc27/src/vs/workbench/api/common/extHost.protocol.ts#L1129
        let defaultRange =
          field.required("a", SuggestItem.SuggestRange.decode);
        {
          completions:
            field.required(
              "b",
              list(SuggestItem.Dto.decode(~defaultRange)),
            ),
          isIncomplete: field.withDefault("c", false, bool),
          cacheId: field.optional("x", int),
        };
      })
    );
  };
};
