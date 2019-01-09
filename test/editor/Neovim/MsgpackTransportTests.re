
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
});
