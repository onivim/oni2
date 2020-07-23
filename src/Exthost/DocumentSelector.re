open Oni_Core;

[@deriving show]
type t = list(DocumentFilter.t);

let decode = Json.Decode.(list(DocumentFilter.decode));

let matches = (~filetype: string, filter) => {
  filter |> List.exists(DocumentFilter.matches(~filetype));
};

let matchesBuffer = (~buffer: Oni_Core.Buffer.t, filter) => {
  buffer
  |> Oni_Core.Buffer.getFileType
  |> Option.map(filetype => matches(~filetype, filter))
  |> Option.value(~default=false);
};
