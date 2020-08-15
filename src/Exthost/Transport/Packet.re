module Log = (val Timber.Log.withNamespace("ExtHost.Transport.Packet"));

module Constants = {
  let headerByteLength = 13;
};

type packetType =
  | Unspecified
  | Regular
  | Control
  | Ack
  | KeepAlive
  | Disconnect;

let typeOfInt =
  fun
  | 0 => Ok(Unspecified)
  | 1 => Ok(Regular)
  | 2 => Ok(Control)
  | 3 => Ok(Ack)
  | 4 => Ok(KeepAlive)
  | 5 => Ok(Disconnect)
  | v => Error(Printf.sprintf("Unknown packet type: %d", v));

let typeToInt =
  fun
  | Unspecified => 0
  | Regular => 1
  | Control => 2
  | Ack => 3
  | KeepAlive => 4
  | Disconnect => 5;

let typeToString =
  fun
  | Unspecified => "Unspecified"
  | Regular => "Regular"
  | Control => "Control"
  | Ack => "Ack"
  | KeepAlive => "KeepAlive"
  | Disconnect => "Disconnect";

// The header for a packet is defined as follows by the VSCode extension host:
// Byte 0 - uint8 - message type
// Byte 1 - uint32 (big endian) - id
// Byte 5 - uint32 (big endian) - ack
// Byte 9 - unit32 (big endian) - data length
module Header = {
  type t = {
    packetType,
    id: int,
    ack: int,
    length: int,
  };

  let ofBytes = bytes =>
    if (Bytes.length(bytes) != Constants.headerByteLength) {
      Error(
        Printf.sprintf("Incorrect header size: %d", Bytes.length(bytes)),
      );
    } else {
      let packetTypeUint = Bytes.get_uint8(bytes, 0);
      let id = Bytes.get_int32_be(bytes, 1) |> Int32.to_int;
      let ack = Bytes.get_int32_be(bytes, 5) |> Int32.to_int;
      let length = Bytes.get_int32_be(bytes, 9) |> Int32.to_int;

      packetTypeUint
      |> typeOfInt
      |> Result.map(packetType => {packetType, id, ack, length});
    };

  let toString = ({packetType, id, ack, length}) =>
    Printf.sprintf(
      "Type: %s Id: %d Ack: %d Length: %d",
      packetType |> typeToString,
      id,
      ack,
      length,
    );

  let toBytes = ({packetType, id, ack, length}) => {
    let bytes = Bytes.create(Constants.headerByteLength);
    let packetTypeInt = packetType |> typeToInt;
    Bytes.set_uint8(bytes, 0, packetTypeInt);
    Bytes.set_int32_be(bytes, 1, id |> Int32.of_int);
    Bytes.set_int32_be(bytes, 5, ack |> Int32.of_int);
    Bytes.set_int32_be(bytes, 9, length |> Int32.of_int);
    bytes;
  };
};

type t = {
  header: Header.t,
  body: Bytes.t,
};

let create = (~bytes: Bytes.t, ~packetType, ~id: int) => {
  let length = Bytes.length(bytes);
  let header =
    Header.{
      packetType,
      id,
      // TODO: Set up ack
      ack: 0,
      length,
    };

  {header, body: bytes};
};

let toBytes = ({header, body}) => {
  let headerBytes = header |> Header.toBytes;
  Bytes.cat(headerBytes, body);
};

let equal = (p1, p2) => {
  let p1Bytes = toBytes(p1);
  let p2Bytes = toBytes(p2);
  Bytes.equal(p1Bytes, p2Bytes);
};

module Parser = {
  type t =
    | WaitingForHeader(ByteWriter.t)
    | WaitingForBody(Header.t, ByteWriter.t);

  type parser = t;

  let initial =
    WaitingForHeader(ByteWriter.create(Constants.headerByteLength));

  let parse = (buffer, initialParser) => {
    let rec loop = (buffer, messages, state) =>
      if (Luv.Buffer.size(buffer) == 0) {
        (state, messages);
      } else {
        let (newState, newBuffer, newMessages) =
          switch (state) {
          | WaitingForHeader(byteWriter) =>
            let (newByteWriter, remainingBuffer) =
              ByteWriter.write(buffer, byteWriter);
            let newState =
              if (ByteWriter.isFull(newByteWriter)) {
                let header =
                  ByteWriter.getBytes(newByteWriter)
                  |> Header.ofBytes
                  |> Result.get_ok;

                let bodyLength = header.length;
                let bodyByteWriter = ByteWriter.create(bodyLength);
                WaitingForBody(header, bodyByteWriter);
              } else {
                WaitingForHeader(newByteWriter);
              };
            (newState, remainingBuffer, messages);
          | WaitingForBody(header, byteWriter) =>
            let (newByteWriter, remainingBuffer) =
              ByteWriter.write(buffer, byteWriter);
            let (newState, messages) =
              if (ByteWriter.isFull(newByteWriter)) {
                let body = ByteWriter.getBytes(newByteWriter);
                let newState =
                  WaitingForHeader(
                    ByteWriter.create(Constants.headerByteLength),
                  );
                let newMessage = {header, body};
                (newState, [newMessage, ...messages]);
              } else {
                (WaitingForBody(header, newByteWriter), messages);
              };
            (newState, remainingBuffer, messages);
          };

        loop(newBuffer, newMessages, newState);
      };

    let (parser, revMessages) = loop(buffer, [], initialParser);
    let messages = List.rev(revMessages);
    (parser, messages);
  };
};
