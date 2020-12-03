module ExtCommand = Command;
open Oni_Core;

[@deriving show({with_path: false})]
type command = {
  id: string,
  title: string,
  tooltip: option(string),
  arguments: list([@opaque] Json.t),
};

module Resource = {
  module Icons = {
    [@deriving show({with_path: false})]
    type t = {
      light: Uri.t,
      dark: Uri.t,
    };
  };

  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    uri: Uri.t,
    // TODO: Bring back
    //icons: Icons.t,
    tooltip: string,
    strikeThrough: bool,
    faded: bool,
  };

  module Splice = {
    type nonrec t = {
      start: int,
      deleteCount: int,
      resources: list(t),
    };
  };

  module Splices = {
    [@deriving show({with_path: false})]
    type t = {
      handle: int,
      resourceSplices: [@opaque] list(Splice.t),
    };
  };

  module Decode = {
    let icons =
      Json.Decode.(
        Pipeline.(
          decode((light, dark) => Icons.{light, dark})
          |> custom(index(0, Uri.decode))
          |> custom(index(1, Uri.decode))
        )
      );

    let resource =
      Json.Decode.(
        Pipeline.
          (
            decode(
              (
                handle,
                uri,
                /*icons,*/ tooltip,
                maybeStrikeThrough,
                maybeFaded,
              ) => {
              let strikeThrough =
                maybeStrikeThrough |> Option.value(~default=false);
              let faded = maybeFaded |> Option.value(~default=false);

              {handle, uri, /*icons,*/ tooltip, strikeThrough, faded};
            })
            |> custom(index(0, int))
            |> custom(index(1, Uri.decode))
            // TODO: Bring back icons
            //|> custom(index(2, icons))
            |> custom(index(3, string))
            |> custom(index(4, nullable(bool)))
            |> custom(index(5, nullable(bool)))
          )
          // TODO: Context value?
      );

    let splice =
      Json.Decode.(
        Pipeline.(
          decode((start, deleteCount, resources) =>
            Splice.{start, deleteCount, resources}
          )
          |> custom(index(0, int))
          |> custom(index(1, int))
          |> custom(index(2, list(resource)))
        )
      );
    let splices =
      Json.Decode.(
        Pipeline.(
          decode((handle, resourceSplices) =>
            Splices.{handle, resourceSplices}
          )
          |> custom(index(0, int))
          |> custom(index(1, list(splice)))
        )
      );
  };

  let decode = Decode.resource;
};

module GroupFeatures = {
  [@deriving show({with_path: false})]
  type t = {hideWhenEmpty: bool};

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {hideWhenEmpty: field.withDefault("hideWhenEmpty", false, bool)}
      )
    );
};

module Group = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    features: GroupFeatures.t,
  };

  let decode =
    Json.Decode.(
      Pipeline.(
        decode((handle, id, label, features) =>
          {handle, id, label, features}
        )
        |> custom(index(0, int))
        |> custom(index(1, string))
        |> custom(index(2, string))
        |> custom(index(3, GroupFeatures.decode))
      )
    );
};

module Decode = {
  let command =
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.required("id", string),
          title: field.required("title", string),
          tooltip: field.optional("tooltip", string),
          arguments: field.withDefault("arguments", [], list(value)),
        }
      )
    );
};

module ProviderFeatures = {
  [@deriving show({with_path: false})]
  type t = {
    hasQuickDiffProvider: bool,
    count: option(int),
    commitTemplate: option(string),
    acceptInputCommand: option(command),
    statusBarCommands: option(list(command)),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          hasQuickDiffProvider:
            field.withDefault("hasQuickDiffProvider", false, bool),
          count: field.optional("count", int),
          commitTemplate: field.optional("commitTemplate", string),
          acceptInputCommand:
            field.optional("acceptInputCommand", Decode.command),
          statusBarCommands:
            field.optional("statusBarCommands", list(Decode.command)),
        }
      )
    );
};
