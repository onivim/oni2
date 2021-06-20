open Oni_Core;

module DirectoryEntry: {
  type t;

  let name: t => string;

  let path: t => FpExp.t(FpExp.absolute);

  let isSymbolicLink: t => bool;

  let isFile: t => bool;

  let isDirectory: t => bool;
};

module Api: {
  let glob:
    (
      ~maxCount: int=?,
      ~includeFiles: string=?,
      ~excludeDirectories: string=?,
      string
    ) =>
    Lwt.t(list(string));

  let rmdir: (~recursive: bool=?, string) => Lwt.t(unit);
  let stat: string => Lwt.t(Luv.File.Stat.t);
  let readdir: string => Lwt.t(list(Luv.File.Dirent.t));
  let readdir2: FpExp.t(FpExp.absolute) => Lwt.t(list(DirectoryEntry.t));
  let readFile: (~chunkSize: int=?, string) => Lwt.t(Bytes.t);
  let writeFile: (~contents: Bytes.t, string) => Lwt.t(unit);
  let rename:
    (~source: string, ~target: string, ~overwrite: bool) => Lwt.t(unit);
  let copy:
    (~source: string, ~target: string, ~overwrite: bool) => Lwt.t(unit);
  let mkdir: string => Lwt.t(unit);
  let mkdirp: FpExp.t(FpExp.absolute) => Lwt.t(unit);

  let mktempdir: (~prefix: string=?, unit) => Lwt.t(string);
  let delete: (~recursive: bool, string) => Lwt.t(unit);

  let openURL: string => bool;
};

module Effect: {
  let openURL: string => Isolinear.Effect.t(_);
  let stat: (string, Unix.stats => 'msg) => Isolinear.Effect.t('msg);
  let statMultiple:
    (list(string), (~exists: bool, ~isDirectory: bool, string) => 'msg) =>
    Isolinear.Effect.t('msg);

  module Dialog: {
    let openFolder:
      (
        ~initialDirectory: string=?,
        option(FpExp.t(FpExp.absolute)) => 'msg
      ) =>
      Isolinear.Effect.t('msg);
  };
};

module Sub: {
  let dir:
    (
      ~uniqueId: string,
      ~toMsg: result(list(DirectoryEntry.t), string) => 'msg,
      FpExp.t(FpExp.absolute)
    ) =>
    Isolinear.Sub.t('msg);
};
