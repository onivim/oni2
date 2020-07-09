open Oni_Core;

type t = {
  uri: Uri.t,
  versionId: int,
  lines: list(string),
  eol: Eol.t,
  modeId: string,
  isDirty: bool,
};

let create =
    (~versionId=0, ~lines=[], ~eol=Eol.default, ~isDirty=false, ~modeId, uri) => {
  uri,
  versionId,
  lines,
  eol,
  modeId,
  isDirty,
};

let encode = ({uri, versionId, lines, eol, modeId, isDirty}) =>
  Json.Encode.(
    obj([
      ("uri", uri |> Uri.encode),
      ("versionId", versionId |> int),
      ("lines", lines |> list(string)),
      ("EOL", eol |> Eol.encode),
      ("modeId", modeId |> string),
      ("isDirty", isDirty |> bool),
    ])
  );
