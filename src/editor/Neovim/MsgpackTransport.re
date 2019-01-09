open Rench;

type onDataEvent = Event.t(Bytes.t);
type writeFunction = Bytes.t => unit;

type t = {
    write: Msgpck.t => unit,   
    onMessage: Event.t(Msgpck.t),
    close: unit => unit
};

let make = (~onData: onDataEvent, ~writeFn: writeFunction, ()) => {
    let onMessage: Event.t(Msgpck.t) = Event.create();
   
    let write = (m: Msgpck.t) => {
        Msgpck.Bytes.to_string(m)  
        |> writeFn;
    };

    let close = Event.subscribe(onData, (bytes) => {
      let (_c, msgs) = Msgpck.Bytes.read_all(bytes);  

      let f = (msg) => Event.dispatch(onMessage, msg);
      List.iter(f, msgs);
    });

    let ret: t = {
        write,
        onMessage,
        close,
    };
    ret;
};
