module Imperative: {
  let stat: string => Lwt.t(Luv.File.Stat.t);
  let readdir: string => Lwt.t(list(Luv.File.Dirent.Kind.t));
  let readFile: string => Lwt.t(Bytes.t);
  let writeFile: (string, Bytes.t) => Lwt.t(unit);
  let rename:
    (~source: string, ~target: string, ~overwrite: bool) => Lwt.t(unit);
  let copy:
    (~source: string, ~target: string, ~overwrite: bool) => Lwt.t(unit);
  let mkdir: string => Lwt.t(unit);
  let delete: (~recursive: bool, ~useTrash: bool, string) => Lwt.t(unit);
};

module Effect: {
  let openURL: string => Isolinear.Effect.t(_);
  let stat: (string, Unix.stats => 'msg) => Isolinear.Effect.t('msg);
  let statMultiple:
    (list(string), (string, Unix.stats) => 'msg) => Isolinear.Effect.t('msg);
};
