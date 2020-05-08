/*
 * ExtHostTransport.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

open Oni_Core;
open Reason_jsonrpc;
open Rench;

module Log = (val Log.withNamespace("Oni2.Extensions.ExtHostTransport"));

module Protocol = ExtHostProtocol;
module Workspace = Protocol.Workspace;
module Configuration = Exthost.Configuration;

type t = unit;

type simpleCallback = unit => unit;

type messageHandler =
  (string, string, list(Yojson.Safe.t)) =>
  result(option(Yojson.Safe.t), string);

let start =
    (
      ~initialConfiguration,
      ~initData,
      ~initialWorkspace=Workspace.empty,
      ~onInitialized=() => (),
      ~onMessage=(_, _, _) => Ok(None),
      ~onClosed=() => (),
      setup: Setup.t,
    ) => {
  ignore(initialConfiguration);
  ignore(initData);
  ignore(initialWorkspace);
  ignore(onMessage);
  ignore(onClosed);
  ignore(setup);
  ();
};

let send = (~msgType=MessageType.requestJsonArgs, v: t, msg: Yojson.Safe.t) => {
  ();
};

let request =
    (~msgType=MessageType.requestJsonArgs, v: t, msg: Yojson.Safe.t, f) => {
  //Lwt.fail(Invalid_Argument("Not implemented"));
  Lwt.fail_with("Not implemented");
};

let close = (v: t) => {
  ();
};
