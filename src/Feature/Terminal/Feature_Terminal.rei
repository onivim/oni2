open Oni_Core;
open Utility;

module ExtHostClient = Oni_Extensions.ExtHostClient;

type terminal = {
  id: int,
  cmd: string,
  rows: int,
  columns: int,
  pid: option(int),
  title: option(string),
};

type t;

let initial: t;

let getNextId: t => int;

let getBufferName: int => string;

let toList: t => list((int, terminal));

type msg =
  | Started({
      id: int,
      cmd: string,
    })
  | Service(Service_Terminal.Msg.t);

let update: (ExtHostClient.t, t, msg) => (t, Isolinear.Effect.t(msg));

let subscription:
  (~workspaceUri: Uri.t, ExtHostClient.t, t) => Isolinear.Sub.t(msg);

let shellCmd: string;
