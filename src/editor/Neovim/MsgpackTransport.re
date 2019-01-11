open Rench;

type onDataEvent = Event.t(Bytes.t);
type writeFunction = Bytes.t => unit;

type t = {
  write: Msgpck.t => unit,
  onMessage: Event.t(Msgpck.t),
  close: unit => unit,
};

let make = (~onData: onDataEvent, ~write: writeFunction, ()) => {
  let onMessage: Event.t(Msgpck.t) = Event.create();

  let writeFn = (m: Msgpck.t) => {
    Msgpck.Bytes.to_string(m) |> write;
  };

  let rec flushMessages = (bytes: Bytes.t) => {
    switch (Msgpck.Bytes.read(bytes)) {
    | (c, msg) =>
      Event.dispatch(onMessage, msg);

      let remainder = Bytes.length(bytes) - c;
      if (remainder > 0) {
        let newBytes = Bytes.sub(bytes, c, remainder);
        flushMessages(newBytes);
      } else {
        Bytes.create(0);
      };
    | exception (Invalid_argument(_)) =>
      prerr_endline("Warning - partial buffer");
      bytes;
    };
  };

  let buffer = Buffer.create(0);
  let close =
    Event.subscribe(
      onData,
      bytes => {
        Buffer.add_bytes(buffer, bytes);
        let contents = Buffer.to_bytes(buffer);
        Buffer.clear(buffer);
        let newBytes = flushMessages(contents);
        Buffer.add_bytes(buffer, newBytes);
      },
    );

  let ret: t = {write: writeFn, onMessage, close};
  ret;
};
