module ByteWriter: {
  type t;

  let create: int => t;

  let isFull: t => bool;

  let write: (Luv.Buffer.t, t) => (t, Luv.Buffer.t);

  let getBytes: t => bytes;
};

module Packet: {
  type packetType =
    | Unspecified
    | Regular
    | Control
    | Ack
    | KeepAlive
    | Disconnect;

  module Header: {
    type t = {
      packetType,
      id: int,
      ack: int,
      length: int,
    };

    let ofBytes: bytes => result(t, string);
    let toBytes: t => bytes;
    let toString: t => string;
  };

  type t = {
    header: Header.t,
    body: Bytes.t,
  };

  let create: (~bytes: Bytes.t, ~packetType: packetType, ~id: int) => t;
  let toBytes: t => bytes;
  let equal: (t, t) => bool;

  module Parser: {
    type parser;

    let initial: parser;

    let parse: (Luv.Buffer.t, parser) => (parser, list(t));
  };
};

[@deriving show]
type msg =
  | Connected
  | Received(Packet.t)
  | Error(string)
  | Disconnected;

type t;

let start: (~namedPipe: string, ~dispatch: msg => unit) => result(t, string);

let connect:
  (~namedPipe: string, ~dispatch: msg => unit) => result(t, string);

let send: (~packet: Packet.t, t) => unit;

let close: t => unit;
