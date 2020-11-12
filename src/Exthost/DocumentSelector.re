open Oni_Core;
open Utility;

[@deriving show]
type t = list(DocumentFilter.t);

let decode = Json.Decode.(list(DocumentFilter.decode));

let matches = (~filetype: string, ~filepath: string, filter) => {
  filter |> List.exists(DocumentFilter.matches(~filetype, ~filepath));
};

let matchesBuffer = (~buffer: Oni_Core.Buffer.t, filter) => {
  let maybeFileType = buffer |> Buffer.getFileType |> Buffer.FileType.toOption;

  let maybeFilePath = buffer |> Buffer.getFilePath;

  OptionEx.map2(
    (filetype, filepath) => {matches(~filetype, ~filepath, filter)},
    maybeFileType,
    maybeFilePath,
  )
  |> Option.value(~default=false);
};
