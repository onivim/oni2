open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Service.Net"));

module Internal = {
  let getTemporaryFilePath = () => {
    open Base.Result.Let_syntax;

    // TODO: De-syncify
    let ret = {
      let%bind tempDir = Luv.Path.tmpdir();
      let%bind (tempFile, _) =
        Luv.File.Sync.mkstemp(tempDir ++ "/oni2-download-XXXXXX");
      Ok(tempFile);
    };

    ret |> Result.map_error(Luv.Error.strerror);
  };
};

module Cache = {
  let cache = Hashtbl.create(32);
  let check = (url: string) => {
    let ret = Hashtbl.find_opt(cache, url);
    switch (ret) {
    | None => Log.tracef(m => m("Cache miss: %s", url))
    | Some(_) => Log.tracef(m => m("Cache hit: %s", url))
    };
    ret;
  };

  let add = (~url: string, dest: string) => {
    Log.tracef(m => m("Adding cache entry: %s -> %s", url, dest));
    Hashtbl.add(cache, url, dest);
  };
};

module Request = {
  let json = (~setup, ~decoder: Json.decoder('a), url) => {
    let promise = NodeTask.run(~args=[url], ~setup, "request.js");
    Lwt.bind(
      promise,
      (output: string) => {
        let result: result('a, string) =
          output
          |> Yojson.Safe.from_string
          |> Json.Decode.decode_value(decoder)
          |> Result.map_error(Json.Decode.string_of_error);

        switch (result) {
        | Ok(v) => Lwt.return(v)
        | Error(msg) => Lwt.fail_with(msg)
        };
      },
    );
  };

  let download = (~dest=?, ~setup, url) => {
    let run = maybeDest => {
      let destResult =
        switch (maybeDest) {
        | None => Internal.getTemporaryFilePath()
        | Some(path) => Ok(path)
        };

      switch (destResult) {
      | Error(msg) => Lwt.fail_with(msg)
      | Ok(dest) =>
        NodeTask.run(~args=[url, dest], ~setup, "download.js")
        |> Lwt.map(_ => {
             Cache.add(~url, dest);
             dest;
           })
      };
    };

    let maybeCacheResult = Cache.check(url);

    switch (maybeCacheResult, dest) {
    // There was a result cached, and dest was not specified - return cached dest
    | (Some(cachedDest), None) => Lwt.return(cachedDest)
    // There was a result cached, and a dest was specified... We don't need
    // a request, we can just copy locally.
    | (Some(cachedDest), Some(dest)) =>
      Service_OS.Api.copy(~source=cachedDest, ~target=dest, ~overwrite=true)
      |> Lwt.map(_ => dest)
    // No cache result - run the request
    | (None, maybeDest) => run(maybeDest)
    };
  };
};
