open Rench;

type onDataEvent = Event.t(Bytes.t);
type writeFunction = Bytes.t => unit;

type t = {
    write: Msgpck.t => unit,   
    onMessage: Event.t(Msgpck.t),
    close: unit => unit
};

let make = (~onData: onDataEvent, ~write: writeFunction, ()) => {
    let onMessage: Event.t(Msgpck.t) = Event.create();
   
    let writeFn = (m: Msgpck.t) => {
        Msgpck.Bytes.to_string(m)  
        |> write;
    };


    let buffer = Buffer.create(0);
    let close = Event.subscribe(onData, (bytes) => {
      Buffer.add_bytes(buffer, bytes);
      let contents = Buffer.to_bytes(buffer);
      switch(Msgpck.Bytes.read_all(contents)) {
      | (_c, msgs) => {
          Buffer.clear(buffer);
          let f = (msg) => Event.dispatch(onMessage, msg);
          List.iter(f, msgs);
      }
      | exception Invalid_argument(_) => {
          /* No-op  - we'll continue reading */
          ();
      }
      };

    });

    let ret: t = {
        write: writeFn,
        onMessage,
        close,
    };
    ret;
};
