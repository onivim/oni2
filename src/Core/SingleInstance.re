
let tryToConnectToExistingServer = (~name, pipe: Luv.Pipe.t) => {
    let (promise, resolver) = Lwt.task();
    Luv.Pipe.connect(pipe, name,
    fun
    | Error(e) =>  {
    prerr_endline ("Connect error!");
    Lwt.failwith(resolver, "Error");
    }
    | Ok () =>  {
        prerr_endline ("Wrote to server");
        let message = Luv.Buffer.from_string("Hello world");
        Luv.Stream.write(pipe, [message], (_result, _bytes_written) => {
            Luv.Stream.shutdown(client, ignore)
        });

        Lwt.wakeup(resolver, ());
        }
     );

    promise
    |> LwtEx.sync;
};

let lock = (
    ~name: string,
    ~arguments: 'a,
    ~serialize: 'a => Bytes.t,
    ~deserialize: Bytes.t => 'a,
    ~firstInstance: 'a => unit,
    ~additionalInstance: 'a => unit,
) => {
    let pipe = Luv.Pipe.init () |> Result.get_ok;
    let tryConnect = tryToConnectToExistingServer(~name="echo-pipe", client);

    switch (tryConnect) {
    | Ok() => "Success!");
    }

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
}
