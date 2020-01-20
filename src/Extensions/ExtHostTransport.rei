/*
 * ExtHostTransport.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 * It is a thin layer above the JSON-RPC protocol.
 *
 */

module Protocol = ExtHostProtocol;
module Workspace = Protocol.Workspace;

type t;

type simpleCallback = unit => unit;

type messageHandler =
  (string, string, list(Yojson.Safe.t)) =>
  result(option(Yojson.Safe.t), string);

/*
  [start] creates an instance of an ExtensionHostTransport,
  and is the entry point for register callbacks (initialization / messages / closed).
 */
let start:
  (
    ~initialConfiguration: Configuration.t,
    ~initData: ExtHostInitData.t,
    ~initialWorkspace: Workspace.t=?,
    ~onInitialized: simpleCallback=?,
    ~onMessage: messageHandler=?,
    ~onClosed: simpleCallback=?,
    Oni_Core_Kernel.Setup.t
  ) =>
  t;

/*
  [send] sends a notification that does not require a response from the extension host
 */
let send: (~msgType: MessageType.t=?, t, Yojson.Safe.t) => unit;

/*
  [request] sends a request that does require a response from the extension host.

  [request(v, requestMessage, jsonToResponse)] sends the request message [requestMessage]
  and, upon response, will run the [jsonToResponse] function to transform from JSON
  to a strongly typed value.
 */
let request:
  (~msgType: MessageType.t=?, t, Yojson.Safe.t, Yojson.Safe.t => 'a) =>
  Lwt.t('a);

/*
  [close] closes the extension host and disposes of the running process
 */
let close: t => unit;
