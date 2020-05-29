open Exthost_Transport;
open Exthost_Extension;

module Log = (val Timber.Log.withNamespace("Exthost.Protocol"));

module ByteParser = {
  exception UInt32ConversionException;

  let int32_unsigned_to_int_exn = i => {
    switch (Int32.unsigned_to_int(i)) {
    | None => raise(UInt32ConversionException)
    | Some(v) => v
    };
  };

  let readUInt8 = bytes => {
    let uint8 = Bytes.get_uint8(bytes, 0);
    let bytes = Bytes.sub(bytes, 1, Bytes.length(bytes) - 1);
    (uint8, bytes);
  };

  let readUInt32: bytes => (int, bytes) =
    bytes => {
      let v = Bytes.get_int32_be(bytes, 0) |> int32_unsigned_to_int_exn;

      let bytes = Bytes.sub(bytes, 4, Bytes.length(bytes) - 4);
      (v, bytes);
    };

  let readShortString = bytes => {
    let len = Bytes.length(bytes);
    let strLength = Bytes.get_uint8(bytes, 0);
    let str = Bytes.sub(bytes, 1, strLength) |> Bytes.to_string;
    let bytes = Bytes.sub(bytes, 1 + strLength, len - 1 - strLength);
    (str, bytes);
  };

  let readLongString: bytes => (string, bytes) =
    bytes => {
      let len = Bytes.length(bytes);
      let strLen = Bytes.get_int32_be(bytes, 0) |> int32_unsigned_to_int_exn;
      let str = Bytes.sub(bytes, 4, strLen) |> Bytes.to_string;
      let bytes = Bytes.sub(bytes, 4 + strLen, len - 4 - strLen);
      (str, bytes);
    };

  let readJSONArgs = bytes => {
    let (rpcId, bytes) = readUInt8(bytes);
    let (method, bytes) = readShortString(bytes);
    let (argsString, _) = readLongString(bytes);
    let json = argsString |> Yojson.Safe.from_string;
    (rpcId, method, json);
  };

  let readMixedArgs = bytes => {
    let (rpcId, bytes) = readUInt8(bytes);
    let (method, bytes) = readShortString(bytes);

    let (arrayLength, bytes) = readUInt8(bytes);

    let argString = 1;
    let argBuffer = 2;
    //let argUndefined = 3;

    let rec loop = (bytes, idx) =>
      if (idx >= arrayLength) {
        [];
      } else {
        let (argType, bytes) = readUInt8(bytes);
        if (argType == argString || argType == argBuffer) {
          let (str, bytes) = readLongString(bytes);
          // It's weird, but... if the argument is a string, we need to parse it as JSON:
          // https://github.com/onivim/vscode-exthost/blob/923c38b016c87a205957456e13c62f8dfd3bdc62/src/vs/workbench/services/extensions/common/rpcProtocol.ts#L710

          // If it isn't meant to be JSON, we'll return it as a `String (which is the closet thing we have to 'buffer')
          let result = argType == argString
            ? Yojson.Safe.from_string(str) : `String(str);
          [result, ...loop(bytes, idx + 1)];
        } else {
          [`Null, ...loop(bytes, idx + 1)];
        };
      };

    let args = `List(loop(bytes, 0));

    (rpcId, method, args);
  };
};

module Message = {
  [@deriving (show, yojson({strict: false}))]
  type ok =
    | Empty
    | Json(Yojson.Safe.t)
    | Bytes(bytes);

  [@deriving (show, yojson({strict: false}))]
  type error =
    | Empty
    | Message(string);

  module Incoming = {
    [@deriving (show, yojson({strict: false}))]
    type t =
      | Connected
      | Initialized
      | Ready
      | Terminate
      | RequestJSONArgs({
          requestId: int,
          rpcId: int,
          method: string,
          args: Yojson.Safe.t,
          usesCancellationToken: bool,
        })
      // TODO
      | RequestMixedArgs
      | Acknowledged({requestId: int})
      | Cancel({requestId: int})
      | ReplyOk({
          requestId: int,
          payload: ok,
        })
      | ReplyError({
          requestId: int,
          payload: error,
        })
      | Unknown(bytes)
      | Disconnected;
  };

  module Outgoing = {
    [@deriving (show, yojson({strict: false}))]
    type t =
      | Initialize({
          requestId: int,
          initData: InitData.t,
        })
      | RequestJSONArgs({
          requestId: int,
          rpcId: int,
          method: string,
          args: Yojson.Safe.t,
          usesCancellationToken: bool,
        })
      | ReplyOKEmpty({requestId: int})
      | ReplyOKJSON({
          requestId: int,
          json: Yojson.Safe.t,
        })
      | ReplyError({
          requestId: int,
          error: string,
        })
      | Terminate;
  };

  // Needs to be in sync with rpcProtocol.t
  let requestJsonArgs = 1;
  let requestJsonArgsWithCancellation = 2;
  let requestMixedArgs = 3;
  let requestMixedArgsWithCancellation = 4;
  let acknowledged = 5;
  //  let cancel = 6;
  let replyOkEmpty = 7;
  let replyOkBuffer = 8;
  let replyOkJSON = 9;
  let replyErrError = 10;
  let replyErrEmpty = 11;

  let ofPacket: Packet.t => result(Incoming.t, string) =
    (packet: Packet.t) => {
      open Incoming;
      let {body, _}: Packet.t = packet;

      let len = Bytes.length(body);

      if (len == 0) {
        Ok(Unknown(body));
      } else if (len == 1) {
        let byte = Bytes.get_uint8(body, 0);
        (
          switch (byte) {
          | 1 => Initialized
          | 2 => Ready
          | 3 => Terminate
          | _ => Unknown(body)
          }
        )
        |> Result.ok;
      } else {
        try({
          let (messageType, bytes) = ByteParser.readUInt8(body);
          let (requestId, bytes) = ByteParser.readUInt32(bytes);
          if (messageType == requestJsonArgs
              || messageType == requestJsonArgsWithCancellation) {
            let usesCancellationToken =
              messageType == requestJsonArgsWithCancellation;
            let (rpcId, method, args) = ByteParser.readJSONArgs(bytes);
            Ok(
              RequestJSONArgs({
                requestId,
                rpcId,
                method,
                args,
                usesCancellationToken,
              }),
            );
          } else if (messageType == requestMixedArgs
                     || messageType == requestMixedArgsWithCancellation) {
            let usesCancellationToken =
              messageType == requestMixedArgsWithCancellation;
            let (rpcId, method, args) = ByteParser.readMixedArgs(bytes);
            Ok(
              RequestJSONArgs({
                requestId,
                rpcId,
                method,
                args,
                usesCancellationToken,
              }),
            );
          } else if (messageType == replyOkEmpty) {
            Ok(ReplyOk({requestId, payload: Empty}));
          } else if (messageType == acknowledged) {
            Ok(Acknowledged({requestId: requestId}));
          } else if (messageType == replyOkBuffer) {
            let (msg, _bytes) = ByteParser.readLongString(bytes);
            Ok(ReplyOk({requestId, payload: Bytes(Bytes.of_string(msg))}));
          } else if (messageType == replyOkJSON) {
            let (msg, _bytes) = ByteParser.readLongString(bytes);
            let json = msg |> Yojson.Safe.from_string;
            Ok(ReplyOk({requestId, payload: Json(json)}));
          } else if (messageType == replyErrEmpty) {
            Ok(ReplyError({requestId, payload: Empty}));
          } else if (messageType == replyErrError) {
            let (msg, _bytes) = ByteParser.readLongString(bytes);
            Ok(ReplyError({requestId, payload: Message(msg)}));
          } else {
            Error("Unknown message - type: " ++ string_of_int(messageType));
          };
        }) {
        | e => Error(Printexc.to_string(e))
        };
      };
    };

  let toPacket = (~id, msg) => {
    open Outgoing;

    let packetId = id;
    let getRequestId =
      fun
      | Initialize({requestId, _}) => requestId
      | RequestJSONArgs({requestId, _}) => requestId
      | ReplyOKEmpty({requestId, _}) => requestId
      | ReplyOKJSON({requestId, _}) => requestId
      | ReplyError({requestId, _}) => requestId
      | Terminate => Int.max_int;
    let requestId = getRequestId(msg) |> Int32.of_int;

    let buffer = Buffer.create(256);

    let writePreamble = (~buffer, ~msgType, ~requestId) => {
      Buffer.add_uint8(buffer, msgType);
      Buffer.add_int32_be(buffer, requestId);
    };

    let writeShortString = (buf, str) => {
      let len = String.length(str);
      Buffer.add_uint8(buf, len);
      Buffer.add_string(buf, str);
    };

    let writeLongString = (buf, str) => {
      let len = String.length(str);
      Buffer.add_int32_be(buf, len |> Int32.of_int);
      Buffer.add_string(buf, str);
    };

    let bufferToPacket = (~buffer) => {
      let bytes = Buffer.to_bytes(buffer);
      Packet.create(~bytes, ~packetType=Packet.Regular, ~id=packetId);
    };

    switch (msg) {
    | Terminate =>
      let bytes = Bytes.make(1, Char.chr(3));
      Packet.create(~bytes, ~packetType=Packet.Regular, ~id=packetId);
    | Initialize({initData, _}) =>
      let str = initData |> InitData.to_yojson |> Yojson.Safe.to_string;

      let bytes = str |> Bytes.of_string;

      Packet.create(~bytes, ~packetType=Packet.Regular, ~id=packetId);
    | RequestJSONArgs({rpcId, method, args, usesCancellationToken, _}) =>
      let msgType =
        usesCancellationToken
          ? requestJsonArgsWithCancellation : requestJsonArgs;
      writePreamble(~buffer, ~msgType, ~requestId);
      Buffer.add_uint8(buffer, rpcId);
      writeShortString(buffer, method);
      let args = Yojson.Safe.to_string(args);
      writeLongString(buffer, args);
      bufferToPacket(~buffer);
    | ReplyOKEmpty(_) =>
      writePreamble(~buffer, ~msgType=replyOkEmpty, ~requestId);
      bufferToPacket(~buffer);
    | ReplyOKJSON({json, _}) =>
      writePreamble(~buffer, ~msgType=replyOkJSON, ~requestId);
      let reply = json |> Yojson.Safe.to_string;
      writeLongString(buffer, reply);
      bufferToPacket(~buffer);
    | ReplyError({error, _}) =>
      writePreamble(~buffer, ~msgType=replyErrError, ~requestId);
      writeLongString(buffer, "\"" ++ error ++ "\"");
      bufferToPacket(~buffer);
    };
  };
};

type t = {
  lastId: ref(int),
  transport: ref(option(Exthost_Transport.t)),
};

let start =
    (
      ~namedPipe: string,
      ~dispatch: Message.Incoming.t => unit,
      ~onError: string => unit,
    ) => {
  let transport = ref(None);

  let onPacket = (packet: Packet.t) =>
    if (packet.header.packetType == Packet.Regular) {
      let message = Message.ofPacket(packet);

      message |> Result.iter(dispatch);

      message |> Result.iter_error(err => {onError(err)});
    };

  let transportHandler = msg =>
    switch (msg) {
    | Exthost_Transport.Error(msg) => onError(msg)
    | Exthost_Transport.Connected => dispatch(Message.Incoming.Connected)
    | Exthost_Transport.Disconnected =>
      dispatch(Message.Incoming.Disconnected)
    | Exthost_Transport.Received(packet) => onPacket(packet)
    };

  let resTransport =
    Exthost_Transport.start(~namedPipe, ~dispatch=transportHandler);

  resTransport |> Result.iter(t => transport := Some(t));

  let lastId = ref(0);
  resTransport |> Result.map(_ => {transport, lastId});
};

let send = (~message: Message.Outgoing.t, {transport, lastId}: t) => {
  transport^
  |> Option.iter(trans => {
       incr(lastId);
       let id = lastId^;
       // Serialize message into packet
       // Send to transport if available
       let packet = message |> Message.toPacket(~id);
       Exthost_Transport.send(~packet, trans);
     });
};

let close = ({transport, _}: t) => {
  transport^
  |> Option.iter(trans => {
       Exthost_Transport.close(trans);
       transport := None;
     });
};
