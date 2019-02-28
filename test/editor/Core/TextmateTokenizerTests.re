open Oni_Core;
open TestFramework;

open Reason_jsonrpc;

describe("Textmate Service", ({test, _}) =>
  test("receive init message", ({expect}) =>
    Helpers.repeat(() => {
      let setup = Setup.init();

      let proc = NodeProcess.start(setup, setup.textmateServicePath);

      let gotInitNotification = ref(false);
      let gotCloseNotification = ref(false);

      let onNotification = (n: Notification.t, _) =>
        switch (n.method) {
        | "initialized" => gotInitNotification := true
        | m => prerr_endline("Unrecognized message: " ++ m)
        };

      let onRequest = (_, _) => Ok(Yojson.Safe.from_string("{}"));

      let onClose = () => gotCloseNotification := true;

      let rpc =
        Rpc.start(
          ~onNotification,
          ~onRequest,
          ~onClose,
          proc.stdout,
          proc.stdin,
        );
      Rpc.sendNotification(rpc, "initialize", Yojson.Safe.from_string("{}"));

      Oni_Core.Utility.waitForCondition(() => {
        Rpc.pump(rpc);
        gotInitNotification^;
      });
      expect.bool(gotInitNotification^).toBe(true);

      Rpc.sendNotification(rpc, "exit", Yojson.Safe.from_string("{}"));

      Oni_Core.Utility.waitForCondition(() => {
        Rpc.pump(rpc);
        gotCloseNotification^;
      });
      expect.bool(gotCloseNotification^).toBe(true);

      let result = Unix.waitpid([], proc.pid);

      switch (result) {
      | (_, Unix.WEXITED(v)) => expect.int(v).toBe(0)
      | _ =>
        expect.string("Expected WEXITED").toEqual("Got different exit state")
      };
    })
  )
);
