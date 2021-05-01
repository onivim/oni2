// SingleInstance
// This is essentially a port of the `node-single-instance` library, into native reason:
// https://github.com/pierrefourgeaud/node-single-instance/blob/master/index.js

exception ConnectError(string);

module Log = (val Kernel.Log.withNamespace("Oni2.SingleInstance"));

module Internal = {
  let tryToConnectToExistingServer = (~name, ~serialize, ~arguments) => {
    let pipe = Luv.Pipe.init() |> Result.get_ok;
    let (promise, resolver) = Lwt.task();
    Luv.Pipe.connect(
      pipe,
      name,
      fun
      | Error(e) => {
          Log.infof(m =>
            m(
              "Unable to connect to existing pipe [%s]: %s",
              name,
              Luv.Error.strerror(e),
            )
          );
          Lwt.wakeup_exn(
            resolver,
            ConnectError("Connect error: " ++ Luv.Error.strerror(e)),
          );
        }
      | Ok () => {
          Log.infof(m => m("Trying to write to server: %s", name));
          let message = arguments |> serialize |> Luv.Buffer.from_bytes;
          Luv.Stream.write(pipe, [message], (_result, _bytes_written) => {
            Luv.Stream.shutdown(pipe, ignore)
          });

          Log.infof(m => m("Wrote to server: %s", name));
          Lwt.wakeup(resolver, ());
        },
    );

    promise |> Utility.LwtEx.sync;
  };

  let tryToCreateServer = (~name, ~onConnected) => {
    let server = Luv.Pipe.init() |> Result.get_ok;
    ignore(Luv.Pipe.bind(server, name));

    Luv.Stream.listen(
      server,
      fun
      | Error(msg) =>
        Log.errorf(m =>
          m("Listen error [%s]: %s", name, Luv.Error.strerror(msg))
        )
      | Ok () => {
          let client = Luv.Pipe.init() |> Result.get_ok;

          switch (Luv.Stream.accept(~server, ~client)) {
          | Error(msg) =>
            Log.errorf(m =>
              m(
                "Error accepting client [%s]: %s",
                name,
                Luv.Error.strerror(msg),
              )
            );
            Luv.Handle.close(client, ignore);
          | Ok () =>
            Luv.Stream.read_start(
              client,
              fun
              | Error(`EOF) => Luv.Handle.close(client, ignore)
              | Error(e) => {
                  Log.errorf(m =>
                    m("Read error[%s]: %s", name, Luv.Error.strerror(e))
                  );
                  Luv.Handle.close(client, ignore);
                }
              | Ok(buf) => {
                  Log.errorf(m =>
                    m("Got %d bytes from %s", Luv.Buffer.size(buf), name)
                  );
                  onConnected(Luv.Buffer.to_bytes(buf));
                },
            )
          };
        },
    );
  };
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
  let name =
    if (Sys.win32) {
      Printf.sprintf("\\\\.\\pipe\\%s%s", name, "-sock");
    } else {
      Filename.get_temp_dir_name() ++ "/name.sock";
    };

  Log.infof(m => m("Using socket: %s", name));
  let tryConnect =
    Internal.tryToConnectToExistingServer(~name, ~arguments, ~serialize);

  switch (tryConnect) {
  | Ok () => Log.infof(m => m("Connected to existing socket: %s", name))
  | Error(msg) =>
    Log.infof(m =>
      m("Unable to connect to exiting server: %s", Printexc.to_string(msg))
    );

    if (!Sys.win32) {
      switch (Luv.File.Sync.unlink(name)) {
      | Ok () => Log.infof(m => m("Deleted file %s", name))
      | Error(msg) =>
        Log.infof(m =>
          m("Unable to delete file %s: %s", name, Luv.Error.strerror(msg))
        )
      };
    };

    let _ =
      Internal.tryToCreateServer(
        ~name,
        ~onConnected=bytes => {
          Log.infof(m =>
            m("Client connected - received %d bytes", Bytes.length(bytes))
          );
          try(bytes |> deserialize |> additionalInstance) {
          | exn =>
            Log.errorf(m =>
              m("error deserializing bytes: %s", Printexc.to_string(exn))
            )
          };
        },
      );
    firstInstance(arguments);
  };
};
