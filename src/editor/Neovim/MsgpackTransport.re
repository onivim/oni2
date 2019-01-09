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

    let close = Event.subscribe(onData, (bytes) => {
      switch(Msgpck.Bytes.read_all(bytes)) {
      | (_c, msgs) => {
          let f = (msg) => Event.dispatch(onMessage, msg);
          List.iter(f, msgs);
      }
      | exception Invalid_argument(_) => {
        prerr_endline ("unable to read");
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
