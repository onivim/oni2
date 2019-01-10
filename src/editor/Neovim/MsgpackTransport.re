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

  let buffer = Buffer.create(0);
  let close =
    Event.subscribe(
      onData,
      bytes => {
          prerr_endline ("MsgpackTransport - got bytes: " ++ string_of_int(Bytes.length(bytes)));
        Buffer.add_bytes(buffer, bytes);
            prerr_endline ("-adding bytes");
        let contents = Buffer.to_bytes(buffer);
            prerr_endline ("-got contents");
        switch (Msgpck.Bytes.read(contents)) {
        | (_c, msg) =>
        /* NEED TO FLUSH ALL CONTENTS - WE MISSED ONE! */
          Buffer.clear(buffer);
        prerr_endline ("-- READ: " ++ string_of_int(_c));

          /* let f = msg => { */
              /* prerr_endline ("Dispatching: " ++ Msgpck.show(msg)); */
              Event.dispatch(onMessage, msg);
          /* }; */
          /* List.iter(f, msg); */
          prerr_endline ("DONE!");
        | exception (Invalid_argument(_)) =>
            prerr_endline ("WARNING: PARTIAL BUFFER!");
          /* No-op  - we'll continue reading */
          ()
        };
      },
    );

  let ret: t = {write: writeFn, onMessage, close};
  ret;
};
