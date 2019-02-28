open Oni_Core;
open TestFramework;

open Reason_jsonrpc;

describe("Textmate Service", ({test, _}) =>
  test("receive init message", ({expect}) => {
    let setup = Setup.init();

    let proc = NodeProcess.start(setup.textmateServicePath);
    /* let (pstdin, stdin) = Unix.pipe(); */
    /* let (stdout, pstdout) = Unix.pipe(); */

    /* Unix.set_close_on_exec(pstdin); */
    /* Unix.set_close_on_exec(stdin); */
    /* Unix.set_close_on_exec(pstdout); */
    /* Unix.set_close_on_exec(stdout); */

    /*   let pid = Unix.create_process_env("node", [|"node", setup.textmateServicePath|], Unix.environment(), pstdin, pstdout, Unix.stderr); */
    /*   let in_channel = Unix.in_channel_of_descr(stdout); */
    /*   let out_channel = Unix.out_channel_of_descr(stdin); */

    /*   Unix.close(pstdout); */
    /*   Unix.close(pstdin); */

    let gotInitNotification = ref(false);

    let onNotification = (n: Types.Notification.t, _) =>
      switch (n.method) {
      | "initialized" =>
        gotInitNotification := true;
        prerr_endline("Setting conditin");
      | m => prerr_endline("Unrecognized message: " ++ m)
      };

    let onRequest = (_, _) => Ok(Yojson.Safe.from_string("{}"));

    let rpc = Rpc.start(~onNotification, ~onRequest, proc.stdout, proc.stdin);
    /* let result = Tokenizer.tokenize(""); */
    /* expect.int(List.length(result)).toBe(0); */
    Rpc.sendNotification(rpc, "initialize", Yojson.Safe.from_string("{}"));
    prerr_endline("test");
    Oni_Core.Utility.waitForCondition(() => gotInitNotification^);

    Rpc.sendNotification(rpc, "exit", Yojson.Safe.from_string("{}"));

    let result = Unix.waitpid([], proc.pid);

    switch (result) {
    | (_, Unix.WEXITED(v)) => expect.int(v).toBe(0)
    | _ =>
      expect.string("Expected WEXITED").toEqual("Got different exit state")
    };
  })
);
