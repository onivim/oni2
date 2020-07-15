open Oni_Core;

type t = (Msg.t => Lwt.t(Reply.t), Msg.t) => Lwt.t(Reply.t);

let filesystem =
    (
      ~stat: Uri.t => Lwt.t(Files.StatResult.t),
      ~readdir: Uri.t => Lwt.t(list((string, Files.FileType.t))),
      ~readFile: Uri.t => Lwt.t(Bytes.t),
      ~writeFile: (Uri.t, Bytes.t) => Lwt.t(unit),
      ~rename:
         (~source: Uri.t, ~target: Uri.t, Files.FileOverwriteOptions.t) =>
         Lwt.t(unit),
      ~copy:
         (~source: Uri.t, ~target: Uri.t, Files.FileOverwriteOptions.t) =>
         Lwt.t(unit),
      ~mkdir: Uri.t => Lwt.t(unit),
      ~delete: (Uri.t, Files.FileDeleteOptions.t) => Lwt.t(unit),
      next: Msg.t => Lwt.t(Reply.t),
      msg,
    ) => {
  let mapEncoder = (encoder, v) => {
    v |> Json.Encode.encode_value(encoder) |> Reply.okJson;
  };

  let mapEmpty = _ => Reply.okEmpty;

  Msg.(
    switch (msg) {
    | FileSystem(fileSystemMsg) =>
      switch (fileSystemMsg) {
      | Stat({uri}) =>
        stat(uri) |> Lwt.map(mapEncoder(Files.StatResult.encode))
      | ReadDir({uri}) => readdir(uri) |> Lwt.map(mapEmpty)
      // TODO: How to serialize buffer?
      | ReadFile({uri}) => readFile(uri) |> Lwt.map(mapEmpty)
      | WriteFile({uri, bytes}) =>
        writeFile(uri, bytes) |> Lwt.map(mapEmpty)
      | Rename({source, target, opts}) =>
        rename(~source, ~target, opts) |> Lwt.map(mapEmpty)
      | Copy({source, target, opts}) =>
        copy(~source, ~target, opts) |> Lwt.map(mapEmpty)
      | Mkdir({uri}) => mkdir(uri) |> Lwt.map(mapEmpty)
      | Delete({uri, opts}) => delete(uri, opts) |> Lwt.map(mapEmpty)
      | _ => next(msg)
      }
    | msg => next(msg)
    }
  );
};
