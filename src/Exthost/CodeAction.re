module StringMap = Oni_Core.StringMap;

[@deriving show]
type t = {
  chainedCacheId: option(ChainedCacheId.t),
  title: string,
  edit: option(WorkspaceEdit.t),
  diagnostics: list(Diagnostic.t),
  command: option(Command.t),
  kind: option(string),
  isPreferred: bool,
  disabled: option(string),
};

module Decode = {
  let decode = {
    Oni_Core.Json.Decode.(
      obj(({field, _}) =>
        {
          chainedCacheId:
            field.optional("chainedCacheId", ChainedCacheId.decode),
          title: field.required("title", string),
          edit: field.optional("edit", WorkspaceEdit.decode),
          diagnostics:
            field.withDefault("diagnostics", [], list(Diagnostic.decode)),
          command: field.optional("command", Command.decode),
          kind: field.optional("kind", string),
          isPreferred: field.withDefault("isPreferred", false, bool),
          disabled: field.optional("disabled", string),
        }
      )
    );
  };
};

module TriggerType = {
  type t =
    | Auto
    | Manual;

  // Must be kept in sync with:
  // https://github.com/onivim/vscode-exthost/blob/0d6b39803352369daaa97a444ff76352d8452be2/src/vs/editor/common/modes.ts#L641
  let toInt =
    fun
    | Auto => 1
    | Manual => 2;

  let encode = triggerType => triggerType |> toInt |> Oni_Core.Json.Encode.int;
};

module Context = {
  type t = {
    // TODO: What is this for?
    only: option(string),
    trigger: TriggerType.t,
  };

  let encode = {
    Oni_Core.Json.Encode.(
      context =>
        obj([
          ("only", context.only |> nullable(string)),
          ("trigger", context.trigger |> TriggerType.encode),
        ])
    );
  };
};

module ProviderMetadata = {
  [@deriving show]
  type t = {
    providedKinds: list(string),
    providedDocumentation: [@opaque] Oni_Core.StringMap.t(Command.t),
  };

  module Documentation = {
    type t = {
      kind: string,
      command: Command.t,
    };

    let decode = {
      Oni_Core.Json.Decode.(
        obj(({field, _}) => {
          let kind = field.required("kind", string);
          let command = field.required("command", Command.decode);
          {kind, command};
        })
      );
    };
  };

  let decode = {
    Oni_Core.Json.Decode.(
      obj(({field, _}) => {
        let providedKinds =
          field.withDefault("providedKinds", [], list(string));
        let documentationList =
          field.withDefault("documentation", [], list(Documentation.decode));

        let providedDocumentation =
          documentationList
          |> List.fold_left(
               (acc, curr: Documentation.t) => {
                 StringMap.add(curr.kind, curr.command, acc)
               },
               StringMap.empty,
             );

        {providedKinds, providedDocumentation};
      })
    );
  };
};

module List = {
  type nonrec t = {
    cacheId: CacheId.t,
    actions: list(t),
  };

  let decode = {
    Oni_Core.Json.Decode.(
      obj(({field, _}) =>
        {
          cacheId: field.required("cacheId", CacheId.decode),
          actions: field.withDefault("actions", [], list(Decode.decode)),
        }
      )
    );
  };

  let toDebugString = ({actions, _}) => {
    actions |> List.map(action => show(action)) |> String.concat("\n");
  };
};
