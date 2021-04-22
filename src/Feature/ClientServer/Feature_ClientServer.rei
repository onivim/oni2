open Oni_Core;

type model;

let create: unit => model;

[@deriving show]
type msg;

type outmsg =
  | Nothing
  | OpenFile({filePath: FpExp.t(FpExp.absolute)})
  | OpenFolder({folderPath: FpExp.t(FpExp.absolute)});

let update: (msg, model) => (model, outmsg);

let pipe: model => Exthost.NamedPipe.t;

let sub: model => Isolinear.Sub.t(msg);

module RemoteCommand: {
  type t;

  let cli: Oni_CLI.t => t;
};

module Client: {let send: (~server: string, RemoteCommand.t) => Lwt.t(unit);};
