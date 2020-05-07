/*
 * ExtHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

open Oni_Core;

module Protocol = ExtHostProtocol;
module Configuration = Exthost.Configuration;
module Workspace = Protocol.Workspace;
module Core = Oni_Core;

module In = Protocol.IncomingNotifications;
module Out = Protocol.OutgoingNotifications;

module Log = (val Log.withNamespace("Oni2.Extensions.ExtHostClient"));

module SCM = ExtHostClient_SCM;
module Terminal = ExtHostClient_Terminal;

type t = ExtHostTransport.t;

type msg =
  | SCM(SCM.msg)
  | Terminal(Terminal.msg)
  | ShowMessage({
      severity: [ | `Ignore | `Info | `Warning | `Error],
      message: string,
      extensionId: option(string),
    })
  | RegisterTextContentProvider({
      handle: int,
      scheme: string,
    })
  | UnregisterTextContentProvider({handle: int})
  | RegisterDecorationProvider({
      handle: int,
      label: string,
    })
  | UnregisterDecorationProvider({handle: int})
  | DecorationsDidChange({
      handle: int,
      uris: list(Uri.t),
    });
