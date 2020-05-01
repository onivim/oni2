  open Oni_Core;
  
  [@deriving yojson({strict: false})]
  type t = {
    uri: Uri.t,
    versionId: int,
    lines: list(string),
    [@key "EOL"]
    eol: Eol.t,
    modeId: string,
    isDirty: bool,
  };

  let create =
      (
        ~versionId=0,
        ~lines=[],
        ~eol=Eol.default,
        ~isDirty=false,
        ~modeId,
        uri
      ) => {
    uri,
    versionId,
    lines,
    eol,
    modeId,
    isDirty,
  };
