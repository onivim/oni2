open Oni_Core;
open TestFramework;

open Helpers;

open Reason_jsonrpc;
open Rench;

describe("textmate", ({test, _}) => {
  test("get init message", ({expect}) => {
    let setup = Setup.init();
  let (pstdin, stdin) = Unix.pipe();
  let (stdout, pstdout) = Unix.pipe();


  Unix.set_close_on_exec(pstdin);
  Unix.set_close_on_exec(stdin);
  Unix.set_close_on_exec(pstdout);
  Unix.set_close_on_exec(stdout);
    let process = Unix.create_process_env("node", [|"node", setup.textmateServicePath|], [||], pstdin, pstdout, Unix.stderr);
    Unix.close(pstdout);
    Unix.close(pstdin);

    let onNotification = (_, _) => print_endline ("NOTIFICATION");
    let onRequest = (_, _) => { 
        print_endline ("REQUEST");
        Ok(Yojson.Safe.from_string("{}"))
    };

    let in_channel = Unix.in_channel_of_descr(stdin);
    let out_channel = Unix.out_channel_of_descr(stdout);

    Rpc.start(~onNotification, ~onRequest, in_channel, out_channel);
    let result = Tokenizer.tokenize("");
    expect.int(List.length(result)).toBe(0);
  });
});
