open Oni_Core;

[@deriving show({with_path: false})]
type command = {
  id: string,
  title: string,
  tooltip: option(string),
  arguments: list([@opaque] Json.t),
};

module Resource = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    uri: Uri.t,
    icons: list(string),
    tooltip: string,
    strikeThrough: bool,
    faded: bool,
  };
};

module ResourceGroup = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    hideWhenEmpty: bool,
    resources: list(Resource.t),
  };
};

module Provider = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    rootUri: option(Uri.t),
    resourceGroups: list(ResourceGroup.t),
    hasQuickDiffProvider: bool,
    count: int,
    commitTemplate: string,
    acceptInputCommand: option(command),
  };
};

module Decode = {
  open Yojson.Safe.Util;

  let resource =
    fun
    | `List([
        `Int(handle),
        uri,
        `List(icons),
        `String(tooltip),
        `Bool(strikeThrough),
        `Bool(faded),
      ]) =>
      Resource.{
        handle,
        uri: Uri.of_yojson(uri) |> Stdlib.Result.get_ok,
        icons:
          List.filter_map(
            fun
            | `String(icon) => Some(icon)
            | _ => None,
            icons,
          ),
        tooltip,
        strikeThrough,
        faded,
      }

    | _ => failwith("Unexpected json for scm resource");

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
