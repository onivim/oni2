/*
 * Merlin.re
 *
 * Core merlin API
 */

  let pendingRequest = ref(false);

  let getErrors =
      (workingDirectory: string, filePath: string, input: array(string), cb) =>
    if (! pendingRequest^) {
      let merlin = MerlinDiscovery.discover(workingDirectory);
      switch (merlin.ocamlMerlinPath) {
      | Some(v) =>
        print_endline("Using path: " ++ v);
        let _ = Thread.create(
          () => {
            pendingRequest := true;
            let (pstdin, stdin) = Unix.pipe();
            let (stdout, pstdout) = Unix.pipe();
            let (stderr, pstderr) = Unix.pipe();

            Unix.set_close_on_exec(pstdin);
            Unix.set_close_on_exec(stdin);
            Unix.set_close_on_exec(pstdout);
            Unix.set_close_on_exec(stdout);
            Unix.set_close_on_exec(pstderr);
            Unix.set_close_on_exec(stderr);

            let pid =
              Unix.create_process(
                v,
                [|v, "single", "errors", "-filename", filePath|],
                pstdin,
                pstdout,
                pstdout,
              );

            Unix.close(pstdout);
            Unix.close(pstdin);
            Unix.close(pstderr);

            let stdIn = Unix.out_channel_of_descr(stdin);
            let stdout = Unix.in_channel_of_descr(stdout);

            let i = ref(0);
            let len = Array.length(input);

            while (i^ < len) {
              output_string(stdIn, input[i^] ++ "\n");
              Thread.yield();
              incr(i);
            };

            close_out_noerr(stdIn);

            let json = Yojson.Safe.from_channel(stdout);
            let result = MerlinProtocol.parse(json);

            switch (result) {
            | Ok(v) =>
              print_endline("RESULT: " ++ Yojson.Safe.to_string(v));
              let errors = MerlinProtocol.errorResult_of_yojson(v);

              switch (errors) {
              | Ok(v) => cb(v)
              | Error(e) => print_endline("ERROR: " ++ e)
              };
            | Error(e) => print_endline("ERROR: " ++ e)
            };

            pendingRequest := false;
          },
          (),
        );

        Ok("winning");
      | None => Error("No merlin instance available")
      };
    } else {
      Error("other request pending");
    };
