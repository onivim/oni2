open Oni_Core;

type model;

let create: unit => model;

[@deriving show]
type msg;

type outmsg =
  | Nothing
  | OpenFilesAndFolders({
      files: list(FpExp.t(FpExp.absolute)),
      folder: option(FpExp.t(FpExp.absolute)),
    });

let update: (msg, model) => (model, outmsg);

let pipe: model => Exthost.NamedPipe.t;

let sub: model => Isolinear.Sub.t(msg);

module Client: {
  let openFiles:
    (~server: string, ~files: list(string), ~folderToOpen: option(string)) =>
    Lwt.t(unit);
};
