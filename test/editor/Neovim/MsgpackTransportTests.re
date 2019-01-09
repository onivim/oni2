
open Rench;

open Oni_Neovim;
open TestFramework;

let noopWrite = (_) => ();

describe("MsgpackTransport", ({test, _}) => {
  test("writing a simple message dispatches message event", ({expect}) => {
      let evt: Event.t(Bytes.t) = Event.create();

      let msgpack = MsgpackTransport.make(~onData=evt, ~write=noopWrite, ());

      let latestMessages: ref(list(Msgpck.t)) = ref([]);
      let _ = Event.subscribe(msgpack.onMessage, (m) => {
          latestMessages := List.append([m], latestMessages^);
      });

      let bytes = Msgpck.List([Msgpck.Int(0), Msgpck.String("Hello World!")])
                  |> Msgpck.Bytes.to_string;

      Event.dispatch(evt, bytes);
      expect.int(List.length(latestMessages^)).toBe(1);
  });

  test("is robust against partial messages", ({expect}) => {
      let evt: Event.t(Bytes.t) = Event.create();

      let msgpack = MsgpackTransport.make(~onData=evt, ~write=noopWrite, ());

      let latestMessages: ref(list(Msgpck.t)) = ref([]);
      let _ = Event.subscribe(msgpack.onMessage, (m) => {
          latestMessages := List.append([m], latestMessages^);
      });

      let bytes = Msgpck.List([Msgpck.Int(0), Msgpck.String("Hello World!")])
                  |> Msgpck.Bytes.to_string;
      let bytes =  Bytes.sub(bytes, 0, 5);


      Event.dispatch(evt, bytes);
      expect.int(List.length(latestMessages^)).toBe(0);
  });

  test("re-constructs partial messages", ({expect}) => {
      let evt: Event.t(Bytes.t) = Event.create();

      let msgpack = MsgpackTransport.make(~onData=evt, ~write=noopWrite, ());

      let latestMessages: ref(list(Msgpck.t)) = ref([]);
      let _ = Event.subscribe(msgpack.onMessage, (m) => {
          latestMessages := List.append([m], latestMessages^);
      });

      let bytes = Msgpck.List([Msgpck.Int(0), Msgpck.String("Hello World!")])
                  |> Msgpck.Bytes.to_string;

      let bytesPart1 =  Bytes.sub(bytes, 0, 5);
      let bytesPart2 = Bytes.sub(bytes, 5, Bytes.length(bytes) - 5);

      Event.dispatch(evt, bytesPart1);
      Event.dispatch(evt, bytesPart2);
      expect.int(List.length(latestMessages^)).toBe(1);
  });

  /* TODO: Is this functionality necessary? 
  /* test("publishes multiple messages", ({expect}) => { */
  /*     let evt: Event.t(Bytes.t) = Event.create(); */

  /*     let msgpack = MsgpackTransport.make(~onData=evt, ~write=noopWrite, ()); */

  /*     let latestMessages: ref(list(Msgpck.t)) = ref([]); */
  /*     let _ = Event.subscribe(msgpack.onMessage, (m) => { */
  /*         latestMessages := List.append([m], latestMessages^); */
  /*     }); */

  /*     let firstMessageBytes = Msgpck.List([Msgpck.Int(0), Msgpck.String("Hello World!")]) */
  /*                 |> Msgpck.Bytes.to_string; */

  /*     let _secondMessageBytes = Msgpck.List([Msgpck.Int(1), Msgpck.String("Hello World again!")]) */
  /*                 |> Msgpck.Bytes.to_string; */

  /*     let buffer = Buffer.create(0); */
  /*     Buffer.add_bytes(buffer, firstMessageBytes); */
  /*     Buffer.add_bytes(buffer, _secondMessageBytes); */
  /*     let bytes = Buffer.to_bytes(buffer); */

  /*     Event.dispatch(evt, bytes); */
  /*     expect.int(List.length(latestMessages^)).toBe(2); */
  /* }); */
});
