module Api: {
  let rmdir: (~recursive: bool=?, string) => Lwt.t(unit);
  let stat: string => Lwt.t(Luv.File.Stat.t);
  let readdir: string => Lwt.t(list(Luv.File.Dirent.t));
  let readFile: string => Lwt.t(Bytes.t);
  let writeFile: (~contents: Bytes.t, string) => Lwt.t(unit);
  let rename:
    (~source: string, ~target: string, ~overwrite: bool) => Lwt.t(unit);
  let copy:
    (~source: string, ~target: string, ~overwrite: bool) => Lwt.t(unit);
  let mkdir: string => Lwt.t(unit);

  let mktempdir: (~prefix: string=?, unit) => Lwt.t(string);
  let delete: (~recursive: bool, string) => Lwt.t(unit);
};

module Effect: {
  let openURL: string => Isolinear.Effect.t(_);
  let stat: (string, Unix.stats => 'msg) => Isolinear.Effect.t('msg);
  let statMultiple:
    (list(string), (string, Unix.stats) => 'msg) => Isolinear.Effect.t('msg);
};
