open Oni_Core;
open Rench;
open TestFramework;

open Helpers;

describe("NeovimProtocol", ({describe,  _}) => {
  describe("buffer updates", ({test, _}) =>
    test("Validate we get a buffer update", ({expect}) =>
      repeat(() =>
        withNeovimProtocol(((api, protocol)) => {
          let gotMatchingNotification = ref(false);

          protocol.bufAttach(0);

          protocol.input("i");
          protocol.input("abc");
          protocol.input("def");
          protocol.input("ghi");

          let _ =
            Event.subscribe(protocol.onNotification, n =>
                switch(n) {
                | BufferLines({ lines, _}) => {
                    if (lines == ["abcdefghi"]) {
                    gotMatchingNotification := true;
                    }
                }
                | _ => ()
                }
            );

          let f = () => {
            api.pump();
            gotMatchingNotification^;
          };

          Utility.waitForCondition(f);

          expect.bool(gotMatchingNotification^).toBe(true);
        })
      )
    )
  );
});
