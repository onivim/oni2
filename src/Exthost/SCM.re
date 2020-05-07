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
  type resource = {
    handle: int,
    resourceUri: Uri.t,
    icons: Icons.t,
    tooltip: string,
    strikeThrough: bool,
    faded: bool,
  };

  module Splice = {
    [@deriving show({with_path: false})]
    type t = {
      start: int,
      deleteCount: int,
      resources: list(resource),
    };
  };

  module Splices = {
    [@deriving show({with_path: false})]
    type t = {
      handle: int,
      resourceSplices: list(Splice.t),
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
        Pipeline.(
          decode((handle, resourceUri, icons, tooltip, strikeThrough, faded) =>
            {handle, resourceUri, icons, tooltip, strikeThrough, faded}
          )
          |> custom(index(0, int))
          |> custom(index(1, Uri.decode))
          |> custom(index(2, icons))
          |> custom(index(3, string))
          |> custom(index(4, bool))
          |> custom(index(5, bool))
        )
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
};

module Decode = {
  open Yojson.Safe.Util;

  let listOrEmpty =
    fun
    | `List(list) => list
    | _ => [];

  let command =
    fun
    | `Assoc(_) as obj =>
      Some({
        id: obj |> member("id") |> to_string,
        title: obj |> member("title") |> to_string,
        tooltip: obj |> member("tooltip") |> to_string_option,
        arguments: obj |> member("arguments") |> listOrEmpty,
      })
    | _ => None;
};
