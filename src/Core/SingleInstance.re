exception ConnectError(string);
let tryToConnectToExistingServer = (~name, pipe: Luv.Pipe.t) => {
  let (promise, resolver) = Lwt.task();
  Luv.Pipe.connect(
    pipe,
    name,
    fun
    | Error(e) => {
        prerr_endline("Connect error!");
        Lwt.wakeup_exn(resolver, ConnectError("Connect error"));
      }
    | Ok () => {
        prerr_endline("Wrote to server");
        let message = Luv.Buffer.from_string("Hello world");
        Luv.Stream.write(pipe, [message], (_result, _bytes_written) => {
          Luv.Stream.shutdown(pipe, ignore)
        });

        Lwt.wakeup(resolver, ());
      },
  );

  promise |> Utility.LwtEx.sync;
};

let tryToCreateServer = (~name, _server) => {
  let server = Luv.Pipe.init() |> Result.get_ok;
  ignore(Luv.Pipe.bind(server, name));

  Luv.Stream.listen(
    server,
    fun
    | Error(msg) =>
      prerr_endline("listen error: " ++ Luv.Error.strerror(msg))
    | Ok () => {
        let client = Luv.Pipe.init() |> Result.get_ok;

        switch (Luv.Stream.accept(~server, ~client)) {
        | Error(_) =>
          prerr_endline("Error accepting client");
          Luv.Handle.close(client, ignore);
        | Ok () =>
          Luv.Stream.read_start(
            client,
            fun
            | Error(`EOF) => Luv.Handle.close(client, ignore)
            | Error(e) => {
                prerr_endline("Read error: " ++ Luv.Error.strerror(e));
                Luv.Handle.close(client, ignore);
              }
            | Ok(buf) =>
              prerr_endline("Got message: " ++ Luv.Buffer.to_string(buf)),
          )
        };
      },
  );
};

let lock =
    (
      ~name: string,
      ~arguments: 'a,
      ~serialize: 'a => Bytes.t,
      ~deserialize: Bytes.t => 'a,
      ~firstInstance: 'a => unit,
      ~additionalInstance: 'a => unit,
    ) => {
  let pipe = Luv.Pipe.init() |> Result.get_ok;
  let tryConnect = tryToConnectToExistingServer(~name, pipe);

  switch (tryConnect) {
  | Ok () => prerr_endline("Connected!")
  | Error(msg) =>
    prerr_endline(
      "Error: " ++ Printexc.to_string(msg),
      // Now, try to create a server, if we're the first one...
    );
    // TODO: Delete pipe on non windows...

    if (!Sys.win32) {
      let _ = Luv.File.Sync.unlink(name);
      ();
    };

    let _ = tryToCreateServer(~name, pipe);
    ();
  };
  // TODO: Fix name
  // let%bind _ = Luv.Pipe.bind(server, "echo-pipe");
  // Luv.Stream.listen(server, fun
  // | Error(msg) => prerr_endline ("Listen error: " ++ Luv.Error.strerror(msg))
  // | Ok() => {
  //     prerr_endline ("Listening!")
  //     let client = Luv.Pipe.init() |> Result.get_ok;
  //     switch (Luv.Stream.accept(~server, ~client)) {
  //     | Error(msg) => prerr_endline ("Accept error: " ++ Luv.Error.strerror(msg));
  //         Luv.Handle.close(client ignore)
  //     | Ok () => Luv.Stream.write(client, [Luv.Buffer.of_string("Hello, world")])
  //     }
  // })
};
