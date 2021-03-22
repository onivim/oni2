open Oni_Core;
open Utility;

[@deriving show]
type t = list(DocumentFilter.t);

let decode = Json.Decode.(list(DocumentFilter.decode));

let matches = (~filetype: string, ~filepath: string, filter) => {
  filter |> List.exists(DocumentFilter.matches(~filetype, ~filepath));
};

let toDebugString = filter => {
  let filterString =
    filter |> List.map(DocumentFilter.toString) |> String.concat("\n");

  Printf.sprintf("DocumentSelector - filters:\n %s", filterString);
};

let matchesBuffer = (~buffer: Oni_Core.Buffer.t, filter) => {
  let maybeFileType = buffer |> Buffer.getFileType |> Buffer.FileType.toOption;

  let maybeFilePath = buffer |> Buffer.getFilePath;

  let ret =
    OptionEx.map2(
      (filetype, filepath) => {matches(~filetype, ~filepath, filter)},
      maybeFileType,
      maybeFilePath,
    )
    |> Option.value(~default=false);

  prerr_endline(
    Printf.sprintf(
      "Matches buffer: %s - %s (%s)",
      Buffer.getFilePath(buffer) |> Option.value(~default="(none)"),
      ret |> string_of_bool,
      toDebugString(filter),
    ),
  );
  ret;
};

