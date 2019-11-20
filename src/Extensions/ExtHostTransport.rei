/*
 * ExtHostTransport.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 * It is a thin layer above the JSON-RPC protocol.
 *
 */

open Oni_Core;

module Protocol = ExtHostProtocol;

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
    ~initData: ExtHostInitData.t=?,
    ~onInitialized: simpleCallback=?,
    ~onMessage: messageHandler=?,
    ~onClosed: simpleCallback=?,
    Setup.t
  ) =>
  t;

/*
  [pump] is called to run all pending messages on the main thread
 */
let pump: t => unit;

/*
  [send] sends a notification that does not require a response from the extension host
 */
let send: (t, Yojson.Safe.t) => unit;

/*
  [request] sends a request that does require a response from the extension host.

  [request(v, requestMessage, jsonToResponse)] sends the request message [requestMessage]
  and, upon response, will run the [jsonToResponse] function to transform from JSON
  to a strongly typed value.
 */
let request: (t, Yojson.Safe.t, Yojson.Safe.t => 'a) => Lwt.t('a);

/*
  [close] closes the extension host and disposes of the running process
 */
let close: t => unit;
