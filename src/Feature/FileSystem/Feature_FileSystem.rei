/**
 * Feature_FileSystem.rei
 *
 * Track virtualized file systems provided by the extension host,
 * and provide effects and subscriptions for interacting with those
 * file systems
 */

open Oni_Core;

type model;

let initial: model;

[@deriving show]
type msg;

module Msg: {
  let exthost:
    (~resolver: Lwt.u(Exthost.Reply.t), Exthost.Msg.FileSystem.msg) => msg;
};

// Handle for a particular file system
type handle;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update: (msg, model) => (model, outmsg);

let getFileSystem: (~scheme: string, model) => option(handle);

module Effects: {
  let readFile:
    (
      ~handle: handle,
      ~uri: Uri.t,
      ~toMsg: result(array(string), string) => 'msg,
      model,
      Exthost.Client.t
    ) =>
    Isolinear.Effect.t('msg);
};
