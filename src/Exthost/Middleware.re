open Oni_Core;

let download = (msg: Msg.DownloadService.msg) => {
  switch (msg) {
  | Download({uri, dest}) =>
    let uri = uri |> Oni_Core.Uri.toString;
    let dest = dest |> Oni_Core.Uri.toFileSystemPath;

    Service_Net.Request.download(~dest, ~setup=Oni_Core.Setup.init(), uri)
    |> Lwt.map(_ => Reply.okEmpty);
  };
};

let filesystem = (msg: Msg.FileSystem.msg) => {
  let mapEncoder = (encoder, v) => {
    v |> Json.Encode.encode_value(encoder) |> Reply.okJson;
  };
  open Msg.FileSystem;

  let fileTypeFromStat: Luv.File.Stat.t => Files.FileType.t =
    (statResult: Luv.File.Stat.t) => {
      Luv.File.(
        Luv.File.Stat.(
          Files.FileType.(
            if (Mode.test([`IFREG], statResult.mode)) {
              File;
            } else if (Mode.test([`IFDIR], statResult.mode)) {
              Directory;
            } else if (Mode.test([`IFLNK], statResult.mode)) {
              SymbolicLink;
            } else {
              Unknown;
            }
          )
        )
      );
    };

  let statToExthostStat: Luv.File.Stat.t => Files.StatResult.t =
    stat => {
      Files.(
        (
          {
            fileType: fileTypeFromStat(stat),
            mtime: stat.mtim.sec |> Signed.Long.to_int,
            ctime: stat.ctim.sec |> Signed.Long.to_int,
            size: stat.size |> Unsigned.UInt64.to_int,
          }: StatResult.t
        )
      );
    };

  let dirEntryToExthostDirEntry:
    Luv.File.Dirent.t => (string, Files.FileType.t) =
    dirent => {
      let fileType =
        switch (dirent.kind) {
        | `FILE => Files.FileType.File
        | `DIR => Files.FileType.Directory
        | `LINK => Files.FileType.SymbolicLink
        | _ => Files.FileType.Unknown
        };

      (dirent.name, fileType);
    };

  switch (msg) {
  | Stat({uri}) =>
    uri
    |> Uri.toFileSystemPath
    |> Service_OS.Api.stat
    |> Lwt.map(statToExthostStat)
    |> Lwt.map(mapEncoder(Files.StatResult.encode))

  | ReadDir({uri}) =>
    uri
    |> Uri.toFileSystemPath
    |> Service_OS.Api.readdir
    |> Lwt.map(List.map(dirEntryToExthostDirEntry))
    |> Lwt.map(mapEncoder(Json.Encode.list(Files.ReadDirResult.encode)))

  | ReadFile({uri}) =>
    uri
    |> Uri.toFileSystemPath
    |> Service_OS.Api.readFile
    |> Lwt.map(Reply.okBuffer)

  | WriteFile({uri, bytes}) =>
    uri
    |> Uri.toFileSystemPath
    |> Service_OS.Api.writeFile(~contents=bytes)
    |> Lwt.map(() => Reply.okEmpty)

  | Rename({source, target, opts}) =>
    let sourcePath = source |> Uri.toFileSystemPath;
    let targetPath = target |> Uri.toFileSystemPath;

    Service_OS.Api.rename(
      ~source=sourcePath,
      ~target=targetPath,
      ~overwrite=opts.overwrite,
    )
    |> Lwt.map(() => Reply.okEmpty);

  | Copy({source, target, opts}) =>
    let sourcePath = source |> Uri.toFileSystemPath;
    let targetPath = target |> Uri.toFileSystemPath;

    Service_OS.Api.rename(
      ~source=sourcePath,
      ~target=targetPath,
      ~overwrite=opts.overwrite,
    )
    |> Lwt.map(() => Reply.okEmpty);

  | Mkdir({uri}) =>
    uri
    |> Uri.toFileSystemPath
    |> Service_OS.Api.mkdir
    |> Lwt.map(() => Reply.okEmpty)

  | Delete({uri, opts}) =>
    uri
    |> Uri.toFileSystemPath
    |> Service_OS.Api.rmdir(~recursive=opts.recursive)
    |> Lwt.map(() => Reply.okEmpty)

  | RegisterFileSystemProvider(_) => Lwt.return(Reply.okEmpty)
  | UnregisterProvider(_) => Lwt.return(Reply.okEmpty)
  | OnFileSystemChange(_) => Lwt.return(Reply.okEmpty)
  };
};
