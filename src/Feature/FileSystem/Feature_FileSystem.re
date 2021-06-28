open Oni_Core;
open Utility;

open Exthost;

module Internal = {
  let mapEncoder = (encoder, v) => {
    v |> Json.Encode.encode_value(encoder) |> Reply.okJson;
  };

  let mapEncoderError = (encoder, v) => {
    v |> Json.Encode.encode_value(encoder) |> Reply.errorJson;
  };

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

  let promiseAndResolverToEffect = (promise, resolver) => {
    Isolinear.Effect.create(~name="Feature_FileSystem.promiseEffect", () => {
      Lwt.on_success(promise, v => Lwt.wakeup(resolver, v));
      Lwt.on_failure(promise, exn => Lwt.wakeup_exn(resolver, exn));
    });
  };
};

type handle = int;

type provider = {
  handle,
  scheme: string,
  capabilities: Exthost.Files.FileSystemProviderCapabilities.t,
};

type model = {providers: list(provider)};

let initial = {providers: []};

[@deriving show]
type msg =
  | Exthost(Exthost.Msg.FileSystem.msg, [@opaque] Lwt.u(Exthost.Reply.t));

module Msg = {
  let exthost = (~resolver, msg) => Exthost(msg, resolver);
};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update = (msg, model) => {
  switch (msg) {
  | Exthost(Stat({uri}), resolver) =>
    let promise =
      Lwt.catch(
        () => {
          uri
          |> Uri.toFileSystemPath
          |> Service_OS.Api.stat
          |> Lwt.map(Internal.statToExthostStat)
          |> Lwt.map(Internal.mapEncoder(Files.StatResult.encode))
        },
        _exn => {
          Exthost.Files.FileSystemError.(
            make(Code.fileNotFound)
            |> Lwt.return
            |> Lwt.map(Internal.mapEncoderError(encode))
          )
        },
      );

    (model, Effect(Internal.promiseAndResolverToEffect(promise, resolver)));

  | Exthost(ReadDir({uri}), resolver) =>
    let promise =
      Lwt.catch(
        () => {
          uri
          |> Uri.toFileSystemPath
          |> Service_OS.Api.readdir
          |> Lwt.map(List.map(Internal.dirEntryToExthostDirEntry))
          |> Lwt.map(
               Internal.mapEncoder(
                 Json.Encode.list(Exthost.Files.ReadDirResult.encode),
               ),
             )
        },
        _exn => {
          Exthost.Files.FileSystemError.(
            make(Code.fileNotFound)
            |> Lwt.return
            |> Lwt.map(Internal.mapEncoderError(encode))
          )
        },
      );

    (model, Effect(Internal.promiseAndResolverToEffect(promise, resolver)));

  | Exthost(ReadFile({uri}), resolver) =>
    let promise =
      Lwt.catch(
        () => {
          uri
          |> Uri.toFileSystemPath
          |> Service_OS.Api.readFile
          |> Lwt.map(Reply.okBuffer)
        },
        _exn => {
          Exthost.Files.FileSystemError.(
            make(Code.fileNotFound)
            |> Lwt.return
            |> Lwt.map(Internal.mapEncoderError(encode))
          )
        },
      );

    (model, Effect(Internal.promiseAndResolverToEffect(promise, resolver)));

  | Exthost(WriteFile({uri, bytes}), resolver) =>
    let maybePath =
      uri |> Uri.toFileSystemPath |> FpExp.absoluteCurrentPlatform;

    let mkdirPromise =
      maybePath
      // Get parent directory
      |> Option.map(FpExp.dirName)
      // ...and make sure it's created
      |> Option.map(path => {Service_OS.Api.mkdirp(path)})
      |> Option.value(~default=Lwt.return());

    let promise =
      mkdirPromise
      |> LwtEx.flatMap(() => {
           uri
           |> Uri.toFileSystemPath
           |> Service_OS.Api.writeFile(~contents=bytes)
           |> Lwt.map(() => Reply.okEmpty)
         });

    (model, Effect(Internal.promiseAndResolverToEffect(promise, resolver)));

  | Exthost(Rename({source, target, opts}), resolver) =>
    let sourcePath = source |> Uri.toFileSystemPath;
    let targetPath = target |> Uri.toFileSystemPath;

    let promise =
      Lwt.catch(
        () => {
          Service_OS.Api.rename(
            ~source=sourcePath,
            ~target=targetPath,
            ~overwrite=opts.overwrite,
          )
          |> Lwt.map(() => Reply.okEmpty)
        },
        _exn => {
          Exthost.Files.FileSystemError.(
            make(Code.fileNotFound)
            |> Lwt.return
            |> Lwt.map(Internal.mapEncoderError(encode))
          )
        },
      );

    (model, Effect(Internal.promiseAndResolverToEffect(promise, resolver)));

  | Exthost(Copy({source, target, opts}), resolver) =>
    let sourcePath = source |> Uri.toFileSystemPath;
    let targetPath = target |> Uri.toFileSystemPath;

    let promise =
      Service_OS.Api.rename(
        ~source=sourcePath,
        ~target=targetPath,
        ~overwrite=opts.overwrite,
      )
      |> Lwt.map(() => Reply.okEmpty);

    (model, Effect(Internal.promiseAndResolverToEffect(promise, resolver)));

  | Exthost(Mkdir({uri}), resolver) =>
    let maybePath =
      uri |> Uri.toFileSystemPath |> FpExp.absoluteCurrentPlatform;

    let promise =
      maybePath
      |> Option.map(Service_OS.Api.mkdirp)
      |> Option.value(
           ~default=
             Lwt.fail_with(
               "Exthost.mkdir - invalid path: " ++ Uri.toString(uri),
             ),
         )
      |> Lwt.map(() => Reply.okEmpty);
    (model, Effect(Internal.promiseAndResolverToEffect(promise, resolver)));

  | Exthost(Delete({uri, opts}), resolver) =>
    let promise =
      uri
      |> Uri.toFileSystemPath
      |> Service_OS.Api.rmdir(~recursive=opts.recursive)
      |> Lwt.map(() => Reply.okEmpty);

    (model, Effect(Internal.promiseAndResolverToEffect(promise, resolver)));

  | Exthost(
      RegisterFileSystemProvider({handle, scheme, capabilities}),
      resolver,
    ) => (
      {providers: [{handle, scheme, capabilities}, ...model.providers]},
      Effect(
        Internal.promiseAndResolverToEffect(
          Lwt.return(Reply.okEmpty),
          resolver,
        ),
      ),
    )

  | Exthost(UnregisterProvider({handle}), resolver) => (
      {
        providers: List.filter(prov => prov.handle != handle, model.providers),
      },
      Effect(
        Internal.promiseAndResolverToEffect(
          Lwt.return(Reply.okEmpty),
          resolver,
        ),
      ),
    )

  | Exthost(OnFileSystemChange(_), resolver) => (
      model,
      Effect(
        Internal.promiseAndResolverToEffect(
          Lwt.return(Reply.okEmpty),
          resolver,
        ),
      ),
    )
  };
};

let getFileSystem = (~scheme, model) => {
  let candidates =
    model.providers |> List.filter(prov => prov.scheme == scheme);
  List.nth_opt(candidates, 0) |> Option.map(prov => prov.handle);
};

module Effects = {
  let readFile = (~handle, ~uri, ~toMsg, _model, client) => {
    Isolinear.Effect.createWithDispatch(~name="test", dispatch => {
      let promise =
        Exthost.Request.FileSystem.readFile(~handle, ~uri, client);

      Lwt.on_success(
        promise,
        str => {
          let lines =
            str |> StringEx.removeWindowsNewLines |> StringEx.splitNewLines;
          dispatch(toMsg(Ok(lines)));
        },
      );
      Lwt.on_failure(promise, exn =>
        dispatch(toMsg(Error(Printexc.to_string(exn))))
      );
    });
  };
};
