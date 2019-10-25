/*
 * Merlin.re
 *
 * Core merlin API
 */

open Oni_Core;

let info = msg => Log.info("[Merlin] " ++ msg);
let error = msg => Log.error("[Merlin] " ++ msg);

let _runMerlinCommand =
    (~workingDirectory, ~filePath, ~fileContents, ~args, cb) => {
  let merlin = MerlinDiscovery.discover(workingDirectory);
  switch (merlin.ocamlMerlinPath) {
  | Some(v) =>
    info("Using path: " ++ v);
    let (pstdin, stdin) = Unix.pipe();
    let (stdout, pstdout) = Unix.pipe();
    let (stderr, pstderr) = Unix.pipe();

    Unix.set_close_on_exec(pstdin);
    Unix.set_close_on_exec(stdin);
    Unix.set_close_on_exec(pstdout);
    Unix.set_close_on_exec(stdout);
    Unix.set_close_on_exec(pstderr);
    Unix.set_close_on_exec(stderr);

    let env = Rench.Environment.getEnvironmentVariables();
    let currentPath =
      switch (Rench.EnvironmentVariables.getValue(env, "PATH")) {
      | None => ""
      | Some(v) => v
      };

    let augmentedPath =
      switch (merlin.ocamlMerlinReasonPath) {
      | None => currentPath
      | Some(v) =>
        currentPath ++ Rench.Path.pathSeparator ++ Rench.Path.dirname(v)
      };

    let arguments =
      List.concat([[v, "single"], args, ["-filename", filePath]])
      |> Array.of_list;

    let _ =
      Unix.create_process_env(
        v,
        arguments,
        [|"PATH=" ++ augmentedPath|],
        pstdin,
        pstdout,
        pstderr,
      );

    Unix.close(pstdout);
    Unix.close(pstdin);
    Unix.close(pstderr);

    let stdIn = Unix.out_channel_of_descr(stdin);
    let stdout = Unix.in_channel_of_descr(stdout);

    let i = ref(0);
    let len = Array.length(fileContents);

    Log.perf("merlin.write", () => {
      while (i^ < len) {
        output_string(stdIn, fileContents[i^]);
        Thread.yield();
        output_string(stdIn, "\n");
        Thread.yield();
        incr(i);
      }
    });

    close_out_noerr(stdIn);

    // This is necessary because of merlin issues:
    // https://github.com/ocaml/merlin/issues/1034
    // https://github.com/ocaml/merlin/issues/714
    // On Windows, we can sometimes get an error prelude when there are PPX involved
    let handleMerlinResponse = chan => {
      let isJson = ref(false);
      let isDone = ref(false);
      let json = ref([]);

      while (! isDone^) {
        switch (input_line(chan)) {
        | exception _ => isDone := true
        | line =>
          if (isJson^) {
            json := [line, ...json^];
          } else if (String.length(line) > 0 && line.[0] == '{') {
            json := [line, ...json^];
            isJson := true;
          }
        };
        Thread.yield();
      };

      json^ |> List.rev |> String.concat("");
    };

    let jsonString =
      Log.perf("merlin.read", () => handleMerlinResponse(stdout));
    close_in_noerr(stdout);
    Unix.close(stderr);
    let json = Yojson.Safe.from_string(jsonString);
    let result = MerlinProtocol.parse(json);
    switch (result) {
    | Ok(v) => cb(v)
    | Error(e) => error(e)
    };
  | None => ()
  };
};

let getErrors =
    (
      workingDirectory: string,
      filePath: string,
      fileContents: array(string),
      cb,
    ) => {
  let callback = json => {
    let errors = MerlinProtocol.errorResult_of_yojson(json);
    switch (errors) {
    | Ok(v) => cb(v)
    | Error(e) => error(e)
    };
  };

  _runMerlinCommand(
    ~workingDirectory,
    ~filePath,
    ~fileContents,
    ~args=["errors"],
    callback,
  );
};

let getCompletions =
    (
      ~workingDirectory,
      ~filePath,
      ~fileContents,
      ~position: Types.Position.t,
      ~prefix,
      cb,
    ) => {
  let callback = json => {
    let completions = MerlinProtocol.completionResult_of_yojson(json);
    switch (completions) {
    | Ok(v) => cb(v)
    | Error(e) => error(e)
    };
  };

  let positionString =
    string_of_int(Types.Index.toInt1(position.line))
    ++ ":"
    ++ string_of_int(Types.Index.toInt0(position.character));

  _runMerlinCommand(
    ~workingDirectory,
    ~filePath,
    ~fileContents,
    ~args=["complete-prefix", "-prefix", prefix, "-position", positionString],
    callback,
  );
};
